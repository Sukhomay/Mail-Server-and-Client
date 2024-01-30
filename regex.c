#include <stdio.h>
#include <stdlib.h>
#include <regex.h>

int main() {
    const char *pattern = "Subject: [^@]*";  // This pattern matches strings containing only digits

    regex_t regex;
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        fprintf(stderr, "Failed to compile regex pattern\n");
        exit(EXIT_FAILURE);
    }

    const char *input = "Subject: 12345ab kv,. vk 863 ^$#7h!";

    // Use regexec to test if the input string matches the regex pattern
    if (regexec(&regex, input, 0, NULL, 0) == 0) {
        printf("String matches the regex pattern\n");
    } else {
        printf("String does not match the regex pattern\n");
    }

    // Free the regex structure
    regfree(&regex);

    return 0;
}
