# Resourceful Docs

## 1. strtok(char *str, const char *delim)
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