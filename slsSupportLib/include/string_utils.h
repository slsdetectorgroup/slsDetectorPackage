namespace sls {

/* Implementation of a safe string copy function for setting fields in 
for example the multi sls detector. It tries to copy the size of the 
destination from the source, stopping on '\0'. 

Warning this would truncate the source string and should be used with care. 
Still this is better than strcpy...
*/
template <size_t array_size>
void strcpy_safe(char (&destination)[array_size], const char *source) {
    strncpy(destination, source, array_size);
    destination[array_size - 1] = '\0';
}


}; // namespace sls
