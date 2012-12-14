#ifndef SVN_INFO_H
#define SVN_INFO_H

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
using namespace std;

/**
 *
 * @short reads svnInfo.txt and populates this class members
 */

class svnInfo{
public:
	/** Constructor : populates the class members from svnInfo.txt*/
	svnInfo(string const path){
		revision=0;
		char fName[100];
		strcpy(fName,path.c_str());
		strcat(fName,"/svnInfo.txt");
		cout<<"filepath:"<<fName<<endl;
		//read from file and populate class
		string sLine,sArgName;
		ifstream inFile;
		inFile.open(fName, ifstream::in);
		if(inFile.is_open())
		{
			while(inFile.good())
			{
				getline(inFile,sLine);
				istringstream sstr(sLine);

				if(sstr.good()){
					sstr>>sArgName;

					if(sArgName=="Path:"){
						if(sstr.good())
							sstr>>Path;
					}
					else if(sArgName=="URL:"){
						if(sstr.good())
							sstr>>URL;
					}
					else if(sArgName=="Repository"){

						if(sstr.good()){
							sstr>>sArgName;

							if(sArgName=="Root:"){
								if(sstr.good())
									sstr>>repositoryRoot;
							}
							else{
								if(sstr.good())
									sstr>>repositoryUUID;
							}
						}
					}
					else if(sArgName=="Revision:"){
						if(sstr.good()){
							int rev=0;
							sstr>>sArgName;
							sscanf(sArgName.c_str(),"%x",&rev);
							revision = ((int64_t)rev);
						}
					}
					else if(sArgName=="NodeKind:"){
						if(sstr.good())
							sstr>>nodeKind;
					}
					else if(sArgName=="Schedule:"){
						if(sstr.good())
							sstr>>schedule;
					}
					else if(sArgName=="Last"){
						if(sstr.good()){
							if(sstr.good()){
								sstr>>sArgName;

								if(sArgName=="Author:"){
									if(sstr.good())
										sstr>>lastChangedAuthor;
								}
								else if(sArgName=="Rev:"){
									if(sstr.good())
										sstr>>lastChangedRev;
								}
								else if(sArgName=="Date:"){
									if(sstr.good())
										sstr>>lastChangedDate;
								}
							}
						}
					}
				}
			}
			inFile.close();
		}else
			cout << "ERROR: Could not open svn Info file: svnInfo.txt" << endl;
	};

	/** Returns revision */
	int64_t getRevision(){return revision;};

private:
	string Path;
	string URL;
	string repositoryRoot;
	string repositoryUUID;
	int64_t revision;
	string nodeKind;
	string schedule;
	string lastChangedAuthor;
	string lastChangedRev;
	string lastChangedDate;
};


#endif
