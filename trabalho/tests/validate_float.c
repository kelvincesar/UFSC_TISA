#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct {
    float value;
    int is_valid;
} StringToFloat;

/
void validate_float (StringToFloat *number, char *str) {
	int len;
	int ret = sscanf(str, "%f %n", &(number->value), &len);
	number->is_valid = (ret && len==strlen(str));
}


int main () { 
   int is_valid = 0;
   char str[30] = "2.01231";
   StringToFloat new_reference;
   validate_float(&new_reference, str);
    printf(" %f is valid? %d\n", new_reference.value, new_reference.is_valid);
   return(0);
}