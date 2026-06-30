#include <stdio.h>
#include "injector/injector.hpp"
#include "injector/npt_injector.hpp"

void welcome()
{
	printf("    QQQQQQQQQQ      \t\tquick injector\n");
    printf("  QQQ        QQQ    \t\tversion 1.0\n");
    printf(" QQ            QQ   \n");
    printf(" QQ            QQ   \n");
    printf(" QQ            QQ   \n");
    printf(" QQ      QQ    QQ   \t\tilizavr/zxsrxt\n");
    printf(" QQ       QQ   QQ   \n");
    printf(" QQ        QQQ QQ   \n");
    printf("  QQQ       QQQQ    \n");
    printf("    QQQQQQQQQ QQ    \n");
    printf("           QQQQ     \n");
    printf("            QQ      \n");
    printf("            QQ      \n");

	printf("\n");
}
void help(char * cmdline)
{
    printf("[help]use %s <package> <activity> <library> <inject mode>\n",cmdline);
    printf("[help]inject modes:\n");
    printf("*\tdl - standart ptrace injection with bypass selinux\n");
    printf("*\trm - ptrace injection with remap\n");
    //printf("*\tnpdl - no ptrace injection(without hide library)\n");
}

int main(int argc, char *argv[]) 
{
	welcome();

	if(argc < 5)
    {
		help(argv[0]);
		return -1;
	}

    if(!strcmp(argv[4],"dl")||!strcmp(argv[4],"rm"))
    {
        c_injector::inject_library(argv[1], argv[2], argv[3], argv[4]);
    }
    else if(!strcmp(argv[4],"npdl"))
    {
        c_npt_injector::inject_library(argv[1], argv[2], argv[3], argv[4]);
    }
    else 
    {
        help(argv[0]);
		return -1;
    }
	return 0;
}
