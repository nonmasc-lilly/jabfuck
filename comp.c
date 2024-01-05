#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAJOR 1
#define MINOR 0
#define PATCH 0

char *compile(const char *src);

int main(int argc, char **argv) {
    FILE *fp;
    char *content, *output;
    const char *ifile, *ofile;
    int i, fsz;
    ofile = "bf.asm";

    if(argc < 2) {
        fprintf(stderr, "invalid arguments\n\t"
                        "expected: %s <input file> [OPTIONS]",
                        argv[0]);
        exit(2);
    }
    
    ifile = argv[1];

    for(i=2; i<argc; i++) {
        if(argv[i][0] == '-') {
            switch(argv[i][1]) {
            case 'h':
            case 'v':
                fprintf(stdout, "jabfuck v%d.%d.%d\n\twritten by"
                                " nonmasc-lilly Jan 2024,"
                                " implementing the brainfuck"
                                " programming language, see:\n\t"
                                "https://esolangs.org/wiki/"
                                "Brainfuck\n",
                                MAJOR, MINOR, PATCH);
            case 'o':
                if(strcmp(ofile, "bf.asm")) {
                    fprintf(stderr, "cannot have more than one"
                                    "output file\n");
                    exit(2);
                }
                ofile = argv[++i];
                break;
            }
        }
    }

    fp = fopen(ifile, "r");
    if(!fp) {
        fprintf(stderr, "could not open: %s\n", ifile);
        exit(3);
    }
    content = malloc(fsz = (fseek(fp, 0L, SEEK_END), ftell(fp)));
    fseek(fp, 0L, SEEK_SET);
    fread(content, 1, fsz, fp);
    fclose(fp);

    output = compile(content);

    fp = fopen(ofile, "w");
    fwrite(output, 1, strlen(output), fp);
    fclose(fp);

    free(output);
    free(content);

    return 0;
}

char *compile(const char *src) {
    char *ret, *tmp;
    const char *si, *template, *boilerplate;
    int counter, counter2, lcount, loop_num;
    loop_num = 0;

    boilerplate =
        "section .text\n"
        "  global _start\n\n"
        "input:\n"
        "  mov rbx,  rax\n"
        "  lea rsi, [rsp+rax]\n"
        "  mov rax,  0\n"
        "  mov rdi,  0\n"
        "  mov rdx,  1\n"
        "  syscall\n"
        "  mov rax,  rbx\n"
        "  ret\n"
        "output:\n"
        "  mov rbx,  rax\n"
        "  lea rsi, [rsp+rax]\n"
        "  mov rax, 1\n"
        "  mov rdi, 1\n"
        "  mov rdx, 1\n"
        "  syscall\n"
        "  mov rax, rbx\n"
        "  ret\n"
        "\n_start:\n"
        "  xor rax, rax\n"
        "  sub rsp, 0xFFFF\n"
        "%s"
        "  mov rax, 60\n"
        "  mov rdi, 0\n"
        "  syscall\n";

    ret = calloc(1,1);

    for(si = src; *si; si++) {
        switch(*si) {
        case '<':
            counter = 1;
            while(*(++si) == '<' || *si == '>')
                counter = *si == '<' ? counter+1 : counter-1;
            if(counter == 1) {
                template = "  dec ax\n";
                ret = realloc(ret, strlen(ret)+strlen(template)+1);
                strcat(ret, template);
            } else {
                template = "  sub ax, %d\n";
                ret = realloc(ret, strlen(ret)+strlen(template)+21);
                tmp = malloc(strlen(template)+21);
                sprintf(tmp, template, counter);
                strcat(ret, tmp);
                free(tmp);
            }
            si--;
            break;
        case '>':
            counter = 1;
            while(*(++si) == '<' || *si == '>')
                counter = *si == '>' ? counter+1 : counter-1;
            if(counter == 1) {
                template = "  inc ax\n";
                ret = realloc(ret, strlen(ret)+strlen(template)+1);
                strcat(ret, template);
            } else {
                template = "  add ax, %d\n";
                ret = realloc(ret, strlen(ret)+strlen(template)+21);
                tmp = malloc(strlen(template)+21);
                sprintf(tmp, template, counter);
                strcat(ret, tmp);
                free(tmp);
            }
            si--;
            break;
        case '+':
            counter = 1;
            while(*(++si) == '+' || *si == '-')
                counter = *si == '+' ? counter+1 : counter-1;
            if(counter == 1) {
                template = "  inc byte [rsp+rax]\n";
                ret = realloc(ret, strlen(ret)+strlen(template)+1);
                strcat(ret, template);
            } else {
                template = "  add [rsp+rax], %d\n";
                ret = realloc(ret, strlen(ret)+strlen(template)+21);
                tmp = malloc(strlen(template)+21);
                sprintf(tmp, template, counter);
                strcat(ret, tmp);
                free(tmp);
            }
            si--;
            break;
        case '-':
            counter = 1;
            while(*(++si) == '+' || *si == '-')
                counter = *si == '-' ? counter+1 : counter-1;
            if(counter == 1) {
                template = "  dec byte [rsp+rax]\n";
                ret = realloc(ret, strlen(ret)+strlen(template)+1);
                strcat(ret, template);
            } else {
                template = "  sub [rsp+rax], %d\n";
                ret = realloc(ret, strlen(ret)+strlen(template)+21);
                tmp = malloc(strlen(template)+21);
                sprintf(tmp, template, counter);
                strcat(ret, tmp);
                free(tmp);
            }
            si--;
            break;
        case ',':
            template = "  call input\n";
            ret = realloc(ret, strlen(ret)+strlen(template)+1);
            strcat(ret, template);
            break;
        case '.':
            template = "  call output\n";
            ret = realloc(ret, strlen(ret)+strlen(template)+1);
            strcat(ret, template);
            break;
        case '[':
            counter2=1;
            for(counter=1; si[counter]; counter++) {
                if(si[counter]=='[') counter2++;
                if(si[counter]==']') counter2--;
            }
            if(counter2 > 0) {
                fprintf(stderr, "expected closing ']' for '[' at"
                                " character %d\n", (int)(si-src));
                exit(1);
            }
            template = ".L%d:\n"
                       "  cmp byte [rsp+rax], 0x0\n"
                       "  je .Lo%d\n";
            ret = realloc(ret, strlen(ret)+strlen(template)+41);
            tmp = malloc(strlen(template)+41);
            sprintf(tmp, template, loop_num+1, loop_num+1);
            loop_num++;
            strcat(ret, tmp);
            free(tmp);
            break;
        case ']':
            lcount   = 0;
            counter2 = 0;
            for(counter=(int)(si-src); counter > 0; counter--) {
                if(*(si-counter) == '[') counter2++, lcount++;
                if(*(si-counter) == ']') counter2--;
            }
            if(counter2 < 0) {
                fprintf(stderr, "expected opening '[' for ']' at"
                                " character %d\n", (int)(si-src));
                exit(1);
            }
            template ="  jmp .L%d\n"
                      ".Lo%d:\n";
            ret = realloc(ret, strlen(ret)+strlen(template)+41);
            tmp = malloc(strlen(template)+41); 
            sprintf(tmp, template, lcount, lcount);
            strcat(ret, tmp);
            free(tmp);
            break;
        }
    }
    tmp = malloc(strlen(boilerplate)+strlen(ret)+1);
    sprintf(tmp, boilerplate, ret);
    free(ret);
    ret = tmp;
    return ret;
}
