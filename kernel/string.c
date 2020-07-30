#include "string.h"
#include "const.h"
#include "types.h"

PUBLIC char* memset(char* buf, char value, int size){
    for(int i = 0; i < size; i++)
        buf[i] = value;
        
    return buf;
}

PUBLIC void memcpy(char* dst, const char* src, int size){
    for(int i = 0; i < size; i++)
        dst[i] = src[i];    
}

/**
 * Compare memory areas.
 * 
 * @param s1  The 1st area.
 * @param s2  The 2nd area.
 * @param n   The first n bytes will be compared.
 * 
 * @return  an integer less than, equal to, or greater than zero if the first
 *          n bytes of s1 is found, respectively, to be less than, to match,
 *          or  be greater than the first n bytes of s2.
 *****************************************************************************/
PUBLIC int memcmp(const void * s1, const void *s2, int n)
{
	if ((s1 == 0) || (s2 == 0)) { /* for robustness */
		return (s1 - s2);
	}

	const char * p1 = (const char *)s1;
	const char * p2 = (const char *)s2;
	int i;
	for (i = 0; i < n; i++,p1++,p2++) {
		if (*p1 != *p2) {
			return (*p1 - *p2);
		}
	}
	return 0;
}
// reverse string, keep the last '\0'.
PUBLIC void reverse_str(char str[], int length){
    int start = 0;
    int end = length - 1;
    while(start < end)
    {
        char tmp = str[start];
        str[start++] = str[end];
        str[end--] = tmp;
    }
}

PUBLIC void strcpy(char* dst, const char* src){
	int i = 0;
    while(src[i] != 0){
        dst[i] = src[i];  
        i++;
    }

}

PUBLIC int strlen(const char* str){
    int len = 0;
    while(str[len]) len++;
    return len;
}

PUBLIC uint32_t digit_count(uint32_t num){
    uint32_t count = 0;
    if(num == 0) return 1;
    while(num > 0){
        num /= 10;
        count ++;
    }

    return count;
}



/*
PUBLIC char* itox( int num, char* str) {
    char *	p = str;
	char	ch;
	int	i;
	bool_t	flag = FALSE;

	*p++ = '0';
	*p++ = 'x';

	if(num == 0){
		*p++ = '0';
	}
	else{	
		for(i=28;i>=0;i-=4){
			ch = (num >> i) & 0xF;
			if(flag || (ch > 0)){
				flag = TRUE;
				ch += '0';
				if(ch > '9'){
					ch += 7;
				}
				*p++ = ch;
			}
		}
	}

	*p = 0;

	return str;
}

*/
