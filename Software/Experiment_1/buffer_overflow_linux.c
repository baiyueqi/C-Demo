#include <stdio.h>
#include <string.h>

int user_auth_level = 0;

int UserVerify(char *input_buf)
{
    char pass_buffer[40];
    int verify_flag = 0;

    strcpy(pass_buffer, input_buf);

    if (strcmp(pass_buffer, "Admin@Sys2026Secure") == 0)
    {
        verify_flag = 1;
        user_auth_level = 1;
    }

    return verify_flag;
}

int SystemLogin()
{
    char user_input[100];
    printf("请输入系统登录密钥：");
    scanf("%s", user_input);

    return UserVerify(user_input);
}

void AdminSystemOperate()
{
    printf("\n====================================\n");
    printf("      管理员权限：系统配置读写\n");
    printf("      日志导出、账户批量管理、后台运维\n");
    printf("====================================\n");
}

void NormalUserOperate()
{
    printf("\n[-] 普通用户权限，仅可查看基础信息\n");
}

int main()
{
    printf("======== 后台运维身份认证系统 V1.0 ========\n");
    int login_ret = SystemLogin();

    if (login_ret == 1)
    {
        printf("[登录成功]\n");
    }
    else
    {
        printf("[登录失败，密钥错误]\n");
    }

    if (user_auth_level >= 2)
    {
        AdminSystemOperate();
    }
    else
    {
        NormalUserOperate();
    }

    return 0;
}
