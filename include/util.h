#ifndef INC_UTIL_H_
#define INC_UTIL_H_

void float_to_byte_array(float f, char *arr);
void float_16_to_byte_array(float f, char *arr);
int min(int a, int b);
int max(int a, int b);
float map(int min, int max, int pos);


#endif /* INC_UTIL_H_ */