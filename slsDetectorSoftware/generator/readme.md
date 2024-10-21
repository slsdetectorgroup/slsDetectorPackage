# Generator
used to generate C++ cli commands. and bash autocompletion scripts.
## Autocomplete

### Overview

Looks through the `dump.json` file for the different values of an enum and stores them in the dictionary `type_values`.  
```sh
# To print the different values for enums 
python gen_commands.py -a
```

also the autocomplete.py generates shell autocompletion scripts for both bash and zsh. It uses the template file `bash_autocomplete.in.sh` and adds the necessary code in an output file `bash_autocomplete.sh` (same for zsh).  
To use the bash autocompletion the   `bash_autocomplete.sh` must be sourced.

```sh
source bash_autocomplete.sh
# g <Tab><Tab> will give the list of all commands
# p 0:<Tab><Tab> will also give the list of all commands 
# g exp<Tab> will autocomplete to g exptime
# g exptime <Tab><Tab> will return "s ms us ns"
# p timing <Tab><Tab> will return "auto,burst_trigger,gating..."
```
  
**Note:**  
The dump.json is the AST of the file `slsDetectorPackage/slsSupportLib/src/ToString.cpp`. 

```sh
# to generate the dump.json file
cd slsSupportLib/src
clang++ -Xclang -ast-dump=json -Xclang -ast-dump-filter -Xclang StringTo -c ToString.cpp -I ../include/ -std=gnu++11 > ../../slsDetectorSoftware/generator/autocomplete/dump.json 
# clang version used: 14.0.0-1ubuntu1.1
```

the `dump.json` file produced by clang is not a correct json file because we used the `-ast-dump-filter`. autocomplete.py can be used to fix the format of `dump.json` and produce a new file called `fixed.json` that is json format.
```
# to convert dump.json into correct json format. 
python autocomplete.py -f # produces/updates the file fixed.json
``` 

### Code components

- `type_values` is a dictionary that contains the different values for commands args. It is populated with enum values that stard with defs:: or slsDetectorDefs:: (eg. defs::burstMode). Also it contains values added manually such as "mv,mV" and those start with special::
- `generate_type_values` parses the AST file to find the part that contains if statements. and extracts the different possible values for an enum. 
- `generate_bash_autocomplete` generates autocompletion scripts for bash or zsh. the difference between zsh and bash scripts can be found in the template files. (bash handles 0:<Tab><Tab> completion differently than zsh more details can be found in the comments of the template files )
- `fix_json` fixes the file 'autocomplete/dump.json' and outputs a new corrected file in 'autocomplete/fixed.json'

## Command Parser

Definitely the most important component of all the generator module. 

command_parser exist to keep the commands.yaml file concise and easy to read and produce a complete version of the commands.yaml for the code generator to work on. 

The goal is that the code generator works on a version of commands.yaml that is easy to iterate over and generate code. 
```
# complete version
some_command:
    help: "do this"
    infer_action: true
    actions:
        GET:
            args:
                - argc: 0
                  function: "getCommand"
                  ...
                - argc: 1
                  function: "getCommand"
                  ...
            detectors:
                MYTHEN3:
                    - argc: 0
                      function: "getCommandMythen"
                      ...
        PUT:
            args:
                - argc: 2
                  function: "setCommand"
                  ...
                - argc: 3
                  function: "setCommand"
                  ...
    
```

the complete vesion can only have `args` or `detectors` field inside an action (GET or PUT). **Each element in the args array have a different argc**. and in each element in the args array we can find all the information needed to generate the code for that one argc case. for example in the code  `if(action == 'GET')` and `if (args.size() == 1)`  then we can generate the code for that one case independetly. 


commands.yaml has a lot on ~inheritance~. examples show best what it is:

> fields insides an action will be passed to args and detectors  

> the extended args for default actions will be used for detectors 

> any field can be overriden 

```
resetdacs:
  help: "[(optional) hard] ..."
  actions:
    PUT:
      function: resetToDefaultDacs
      require_det_id: true
      output: [ '"successful"' ]
      input_types: [ bool ]
      args:
        - argc: 1
          arg_types: [ special::hard ]
          input: [ '"1"' ]
        - argc: 0
          input: [ '"0"' ]

# this will be converted to 

resetdacs:
  actions:
    PUT:
      args:
      - arg_types:
        - special::hard
        argc: 1
        cast_input:
        - false
        check_det_id: false
        convert_det_id: true
        function: resetToDefaultDacs
        input:
        - '"1"'
        input_types:
        - bool
        output:
        - '"successful"'
        require_det_id: true
        store_result_in_t: false
      - arg_types: []
        argc: 0
        cast_input:
        - false
        check_det_id: false
        convert_det_id: true
        function: resetToDefaultDacs
        input:
        - '"0"'
        input_types:
        - bool
        output:
        - '"successful"'
        require_det_id: true
        store_result_in_t: false
  command_name: resetdacs
  function_alias: resetdacs
  help: "[(optional) hard] ..."
  infer_action: true

```

command_parser does not have a specific schema for the commands.yaml this is by design so it can be very extensible and future-proof. This also can have problems when there is typos (writing intput instead of input...)

command_parser first verifies the commands.yaml and checks if there's some obvious problems in it. 

templates found in commands.yaml were taken from the CmdProxy code they were added for debugging purposes when writing the generator. 


tricky things:
--
- if input array has n elements and cast_input array is empty. command_parser will fill it with n false values.
- store_result_in_t will be added by default as true to GET action. but as false to PUT action. (unless it is written in the action)
- infer_action by default is true
- commands that have is_description true won't be verified 
- function_alias is the name of the function in the c++ code. by default it is the command name. errors came up with the command virtual as virtual is a reserved keyword in c++
- command_name is the string of the command that will be typed in cli. (frames, exptime, ...). by default it is the command name. pattern is a special keyword in yaml. problems came up with the command pattern
- arg_types is by default input_types unless otherwise specified
- when the parent has specific detector behaviour and the child does not. writing an empty detector section in the action would not inherit any detector specific fields (check exptime1)
- commands can inherit other commands (check exptime1 and exptime2)
- argc: -1 means that the command has an unknown number of arguments


### Code Walkthrough
the code is well commented it is well explained in the script

### Tests
tests for command_parser can be found in `generator/tests/command_parser/` 
```
pip install -r requirements.txt
python -m pytests
```

verification is not well tested 



## codegen

Now for C++ code generation. After parsing the commands.yaml file and producing the extended_commands.yaml `gen_commands.py` will iterate over the commands and generate `Caller.h`, `Caller.cpp`, `inferAction.cpp` and `inferAction.h` .  

### infer action

the generated code will produce 5 new targets: "sls_detector_get sls_detector_put sls_detector_acquire sls_detector_help sls_detector"

`sls_detector_get` will set the action as GET  
`sls_detector_put` will the action as PUT

`sls_detector` will guess the action depending on the number of arguments

the codegen module will generate a function for every command that will return the action based on the number of arguments

```cpp
int InferAction::activate() {

  if (args.size() == 0) {
    return slsDetectorDefs::GET_ACTION;
  }

  if (args.size() == 1) {
    return slsDetectorDefs::PUT_ACTION;
  } else {

    throw RuntimeError("Could not infer action: Wrong number of arguments");
  }
}
```

the `inferAction` class will be called from  `CmdApp.cpp` to infer the action and the command function will be called with the appropriate action.

some commands have the same number of argument count for both get and put. These commands can be found using the the `check_infer.py` script. in the generated code it will say that "sls_detector is disabled"
```bash
# to see these commands
python infer_action/check_infer.py 
```

### Caller.cpp code

in this level we only use the extended_commands.yaml file. 
the `generate()` function in `gen_commands.py` will iterate over all of the commands and :
- write the function signature
- write the help
- write c++ code to check the inputs: check argument count and check if we are able to convert the arguments into the required types
- iterate over actions and arguments
- iterate over the detectors and write code for each one of them (if mythen3 ... if eiger ... else default code... ) and call `codegen.write_arg()` to write the argument for a single argument 

codegen.write_arg()
-
write_arg in codegen reads the argument fields and generate c++ code accordingly.

## fields explanations
- arg_types:[array of types] it is only used for autocompletion no C++ code is dependent on it
- is_description:[boolean] same as above
- template:[boolean] only used in commands.yaml and it won't present in extended_commands.yaml. it is inspired by the CmdProxy.h code 
- help:[string] command help
- input:[array of variable names] the input arguments that will be passed to the function
- input_types:[array of types] the types of the input arguments given to the function
- cast_input:[array of boolean] if true it will cast the corresponding input to the type in input_types
- output:[array] outputs that will be printed (eg. ["123", "'a'"] will be os<<123<<'a')
- function: the function that will be called
- function_alias: the name of the function in the c++ code (more on it in tricky things)
- command_name: the string of the command that will be typed in cli.  (more on it in tricky things)
- require_det_id: if true it will require a detector id to be passed as the last argument
- check_det_id: if true it will check the detector id and throw an error if it is not valid
- convert_det_id: if true it will convert the detector id to the correct type `std::vector<int>{ det_id }`
- store_result_in_t: if true it will store the result of the function in the variable t (more on it in tricky things)
- infer_action: if true it will infer the action (only if sls_detector is used)
- detectors: the detectors that have specific behaviour
- args: the arguments of the command
- argc: the number of arguments
- extra_variables[array]: each element takes three parameters: value, name, type and creates that variable in the beginning of the argument code
- exceptions[array]: each element takes two parameters: condition, message
- pattern_command: takes three arguments: nGetArgs, nPutArgs and command_name and it will write this code
  ```cpp
  int level = -1, iArg = 0, nGetArgs = $nGetArgs$, nPutArgs = $nPutArgs$;
  GetLevelAndUpdateArgIndex(action, $command_name$, level, iArg, nGetArgs,nPutArgs);
  ```
- separate_time_units: takes three parameters: input, output[0], output[1] each one is a variable name
  ```cpp
  std::string tmp_time($input$);
  std::string $output[1]$ = RemoveUnit(tmp_time);
  auto $output[0]$ = StringTo<time::ns>(tmp_time, $output[1]$);
  ```
- convert_to_time: takes three parameters: input[0], input[1], output
  ```cpp
  auto output = StringTo<time::ns>(input[0], input[1]);
  ```
- ctb_output_list: **maybe it should be removed?** takes 5 parameters: GETFCNLIST, GETFCNNAME, GETFCN, suffix, printable_name







