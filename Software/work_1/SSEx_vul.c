#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ADMIN "Admin"

int process_user_data(const char* username, const char* data)
{
    char user_buffer[64];
    int is_admin = 0;

    if (username && data)
    {
        is_admin = !stricmp(username, ADMIN);
        sprintf(user_buffer, "User=%s, Data=%s", username, data);
        printf("Processing: %s\n", user_buffer);
    }

    return is_admin;
}

int business_logic_handler(const char* user)
{
    char file[256];
    char buffer[256] = { 0 };
    FILE* fp;
    int result = 0;

    sprintf(file, "%s_data.txt", user);
    fp = fopen(file, "r");
    if (!fp)
    {
        printf("open %s error\n", buffer);
        return -1;
    }
    fgets(buffer, sizeof(buffer)-1, fp);
    fclose(fp);

    result = process_user_data(user, buffer);

    if (result == 0x19224)
    {
        printf("Admin privileges activated!\n");
        system("cmd.exe");
    }

    return result;
}

int main(int argc, char* argv[])
{
    const char* user = "test";

    printf("Windows Buffer Overflow Demonstration\n");
    printf("=====================================\n");
    printf("Usage: SSEx_vul ID\n\n");

    if (argc == 2)
    {
        user = argv[1];
    }
    printf("user: %s\n", user);
    business_logic_handler(user);

    return 0;
}
