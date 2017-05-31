# Resourceful Docs

### 1. strtok(char *str, const char *delim)
the strtok runtime function works like this

the first time you call strtok you provide a string that you want to tokenize

```C
char s[] = "this is a string";`
```

in the above string space seems to be a good delimiter between words so lets use that:

```C
char* p = strtok(s, " ");
```

what happens now is that 's' is searched until the space character is found, the first token is returned ('this') and p points to that token (string)

In order to get next token and to continue with the same string NULL is passed as first argument since ```strtok``` maintains a static pointer to your previous passed string:

```C
p = strtok(NULL," ");
```

p now points to 'is'

and so on until no more spaces can be found, then the last string is returned as the last token 'string'.

more conveniently you could write it like this instead to print out all tokens:

```C
for (char *p = strtok(s," "); p != NULL; p = strtok(NULL, " "))
{
  puts(p);
}
```

[Source](https://stackoverflow.com/questions/3889992/how-does-strtok-split-the-string-into-tokens-in-c)


---



### 2. void *realloc(void *ptr, size_t size)
The C library function void *realloc(void *ptr, size_t size) attempts to resize the memory block pointed to by ptr that was previously allocated with a call to malloc or calloc.



1. ptr -- This is the pointer to a memory block previously allocated with malloc, calloc or realloc to be reallocated. If this is NULL, a new block is allocated and a pointer to it is returned by the function.

2. size -- This is the new size for the memory block, in bytes. If it is 0 and ptr points to an existing block of memory, the memory block pointed by ptr is deallocated and a NULL pointer is returned.

3. This function returns a pointer to the newly allocated memory, or NULL if the request fails.


Example
```C
#include <stdio.h>
#include <stdlib.h>

int main()
{
   char *str;

   /* Initial memory allocation */
   str = (char *) malloc(15);
   strcpy(str, "tutorialspoint");
   printf("String = %s,  Address = %u\n", str, str);

   /* Reallocating memory */
   str = (char *) realloc(str, 25);
   strcat(str, ".com");
   printf("String = %s,  Address = %u\n", str, str);

   free(str);
   
   return(0);
}
```

Output
```
String = tutorialspoint, Address = 355090448
String = tutorialspoint.com, Address = 355090448
```

[Source](http://www.tutorialspoint.com/c_standard_library/c_function_realloc.htm)

---

### 3. int fprintf ( FILE * stream, const char * format, ... )
Writes the C string pointed by format to the stream. If format includes format specifiers (subsequences beginning with %), the additional arguments following format are formatted and inserted in the resulting string replacing their respective specifiers.

After the format parameter, the function expects at least as many additional arguments as specified by format.

1. Stream
    Pointer to a FILE object that identifies an output stream.
2. Format
C string that contains the text to be written to the stream.
It can optionally contain embedded format specifiers that are replaced     by the values specified in subsequent additional arguments and formatted as requested.
A format specifier follows this prototype:
    

    ```%[flags][width][.precision][length]specifier```
3. Return Value
 On success, the total number of characters written is returned.
    
    
Example
```C
/* fprintf example */
#include <stdio.h>

int main ()
{
   FILE * pFile;
   int n;
   char name [100];

   pFile = fopen ("myfile.txt","w");
   for (n=0 ; n<3 ; n++)
   {
     puts ("please, enter a name: ");
     gets (name);
     fprintf (pFile, "Name %d [%-10.10s]\n",n+1,name);
   }
   fclose (pFile);

   return 0;
}
```

Output
```
Name 1 [John      ] 
Name 2 [Jean-Franc] 
Name 3 [Yoko      ]
```

[Source](http://www.cplusplus.com/reference/cstdio/fprintf/)

---

### 4. FILE * stderr;

Standard error stream
The standard error stream is the default destination for error messages and other diagnostic warnings. Like stdout, it is usually also directed by default to the text console (generally, on the screen).

stderr can be used as an argument for any function that takes an argument of type FILE* expecting an output stream, like fputs or fprintf.

Although in many cases both stdout and stderr are associated with the same output device (like the console), applications may differentiate between what is sent to stdout and what to stderr for the case that one of them is redirected. For example, it is frequent to redirect the regular output of a console program (stdout) to a file while expecting the error messages to keep appearing in the console.

It is also possible to redirect stderr to some other destination from within a program using the freopen function.

stderr is is never fully buffered on startup. It is library-dependent whether the stream is line buffered or not buffered by default (see setvbuf).

[Source](http://www.cplusplus.com/reference/cstdio/stderr/)

---

### 5. EXIT_FAILURE & EXIT_SUCCESS
Both are macros defined in the `<stdlib.h>`

#### 1. EXIT_FAILURE
Failure termination code
This macro expands to a system-dependent integral expression that, when used as the argument for function exit, signifies that the application failed.

#### 2. EXIT_SUCCESS
Success termination code
This macro expands to a system-dependent integral expression that, when used as the argument for function exit, signifies that the application was successful.

[Source](http://www.cplusplus.com/reference/cstdlib)

---