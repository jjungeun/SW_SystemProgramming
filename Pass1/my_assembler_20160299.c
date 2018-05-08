/*
 * 파일명: my_assembler_20160299.c
 * 설 명: 이 프로그램은 이 프로그램은 SIC/XE 머신을 위한 간단한 Assembler 프로그램의 메인루틴으로,
 * 입력된 파일의 코드 중, 명령어에 해당하는 OPCODE를 찾아 출력한다.
 * 파일 내에서 사용되는 문자열 "00000000"에는 자신의 학번을 기입한다.
 */

//프로그램의 헤더를 정의
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "my_assembler_20160299.h"

/* ----------------------------------------------------------------------------------
 * 설명 : 사용자로 부터 어셈블리 파일을 받아서 명령어의 OPCODE를 찾아 출력한다.
 * 매계 : 실행 파일, 어셈블리 파일 
 * 반환 : 성공 = 0, 실패 = < 0 
 * 주의 : 현재 어셈블리 프로그램의 리스트 파일을 생성하는 루틴은 만들지 않았다. 
 *		   또한 중간파일을 생성하지 않는다. 
 * ----------------------------------------------------------------------------------
 */

int main(int args, char *arg[]){
    if(init_my_assembler()<0){
        printf("init_my_assembler : 프로그램 초기화에 실패했습니다.\n");
        return -1;
    }

    if(assem_pass1()<0){
        printf("assem_pass1: 패스1 과정에서 실패했습니다.\n");
        return -1;
    }

    //make_opcode_output("output_20160299.txt");
    make_opcode_output(NULL);
    //make_symtab_output("symtab_20160299.txt");
    /*
    if(assem_pass2()<0){
        printf("assem_pass2: 패스2 과정에서 실패했습니다.\n");
        return -1;
    }

    make_objectcode_output("output_20160299.txt");
    */

   // malloc으로 동적할당한 메모리 해제시키기
   // free_malloc();
    return 0;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 프로그램 초기화를 위한 자료구조 생성 및 파일을 읽는 함수이다. 
 * 매계 : 없음
 * 반환 : 정상종료 = 0 , 에러 발생 = -1
 * 주의 : 각각의 명령어 테이블을 내부에 선언하지 않고 관리를 용이하게 하기 
 *		   위해서 파일 단위로 관리하여 프로그램 초기화를 통해 정보를 읽어 올 수 있도록
 *		   구현하였다. 
 * ----------------------------------------------------------------------------------
 */
int init_my_assembler(void){
    int result;

    if((result = init_inst_file("inst.data"))<0)
        return -1;
    if((result = init_input_file("input.txt"))<0)
        return -1;
    return result;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 머신을 위한 기계 코드목록 파일을 읽어 기계어 목록 테이블(inst_table)을 
 *        생성하는 함수이다. 
 * 매계 : 기계어 목록 파일
 * 반환 : 정상종료 = 0 , 에러 < 0 
 * 주의 : 기계어 목록파일 형식은 자유롭게 구현한다. 예시는 다음과 같다.
 *	
 *	===============================================================================
 *		   | 이름 | 형식 | 기계어 코드 | 오퍼랜드의 갯수 | NULL|
 *	===============================================================================	   
 *		
 * ----------------------------------------------------------------------------------
 */

int init_inst_file(char *inst_file){
    FILE *file;
    int errno = 0;
    file = fopen(inst_file,"rt");

    //구조체 포인터 배열에 메모리 할당
    for(int j=0;j<sizeof(inst_table)/sizeof(inst *);j++){
        inst_table[j]=(inst*)malloc(sizeof(inst));
    }
    
    if(file == NULL){
        errno = -1;
    }
    
    else{
        while(EOF != fscanf(file,"%s %d %s %d",inst_table[inst_index]->mnemonic,
                                                &inst_table[inst_index]->format,
                                                inst_table[inst_index]->opcode,
                                                &inst_table[inst_index]->opernum)){
            inst_index++;
        }
    }

    fclose(file);
    return errno;
}


/* ----------------------------------------------------------------------------------
 * 설명 : 어셈블리 할 소스코드를 읽어 소스코드 테이블(input_data)를 생성하는 함수이다. 
 * 매계 : 어셈블리할 소스파일명
 * 반환 : 정상종료 = 0 , 에러 < 0  
 * 주의 : 라인단위로 저장한다.
 *		
 * ----------------------------------------------------------------------------------
 */
int init_input_file(char *input_file){
    FILE *file;
    int errno = 0;
    file = fopen(input_file,"rt");

    if(file == NULL){
        errno = -1;
    }

    else{
        while(1){
            input_data[line_num] = (char *)malloc(100); //라인 수 만큼만 동적할당
            if(fgets(input_data[line_num],100,file) == NULL) 
               break;
            line_num++;
        }
    }
    fclose(file);
    return errno;
}
/* ----------------------------------------------------------------------------------
 * 설명 : 소스 코드를 읽어와 토큰단위로 분석하고 토큰 테이블을 작성하는 함수이다. 
 *        패스 1로 부터 호출된다. 
 * 매계 : 파싱을 원하는 문자열  
 * 반환 : 정상종료 = 0 , 에러 < 0 
 * 주의 : my_assembler 프로그램에서는 라인단위로 토큰 및 오브젝트 관리를 하고 있다. 
 * ----------------------------------------------------------------------------------
 */
int token_parsing(char *str){
    int errno = 0;
    char *ptr;
    char *operandtmp = NULL;
    char *tmp=NULL;
    char *extreftmp = NULL;
    char *extdeftmp = NULL;
    int refnum = 1;
    int defnum = 1;
    
     //.으로 시작하는 주석은 token_table에 넣지 않음.
    if(str[0]=='.')
        return errno;

    if(str == NULL){
        errno = -1;
    }
    else{
        token_table[token_line] = (token *)malloc(sizeof(token));

        //label이 없는 경우 다음 토큰으로
        if(str[0] == '\t'){
            ptr = strtok(str,"\t");
        }
        else{
            ptr = strtok(str,"\t");
            token_table[token_line]->label = ptr;
            ptr = strtok(NULL,"\t");
       }

        //명령어가 없는 경우는 없음
        token_table[token_line]->oper = ptr;

        //EXTREF
        if(strcmp(token_table[token_line]->oper,"EXTREF")){
            ptr = strtok(NULL,"\t");
            extreftmp = ptr;
        }

        //EXTDEF
        else if(strcmp(token_table[token_line]->oper,"EXTDEF")){
            ptr = strtok(NULL,"\t");
            extdeftmp = ptr;
        }

        //LTORG인 경우 새로운 line추가
        else if(strcmp(token_table[token_line]->oper,"LTORG")){
            ptr = strtok(NULL,"\t");
        }

        else{
            ptr = strtok(NULL,"\t");
        }

        //명령어를 끝으로 다음줄로 넘어가는 경우
        if(ptr == NULL){
            token_line++;
            return errno;
        }

        if(strcmp(token_table[token_line]->oper,"RSUB") == 0){
            strtok(NULL,"\t");
            token_table[token_line]->comment = ptr;
            printf("%s\n",token_table[token_line]->comment);
            token_line++;
            return errno;
        }
        else{
            operandtmp = ptr;
            strtok(NULL,"\t");
            if(ptr != NULL){
                token_table[token_line]->comment = ptr;     //주석 있는 경우
            }
            
            //operand 넣기
            token_table[token_line] -> operand[0] = strtok(operandtmp,",");
            for(int i=1;i<MAX_OPERAND;i++){
                tmp = strtok(NULL,",");
                if(tmp == NULL){
                    refnum = defnum = i;
                    break;          //operand 더이상 없는 경우
                }
                token_table[token_line] -> operand[i] = tmp;
            }

            //EXTREF 저장하기
            if(extreftmp != NULL){
                extRef[0] =  strtok(extreftmp,",");
                for(int i=1;i<refnum;i++){
                    extRef[i] = strtok(NULL,",");
                    printf("%s\n",*extRef[i]);
                }
            }
            //EXTDEF 저장하기
            else if(extdeftmp != NULL){
                extDef[0] = strtok(extdeftmp,",");
                for(int i=1;i<defnum;i++){
                    extDef[i] = strtok(NULL,",");
                }
            }
        }
        token_line++;
    }
    return errno;
}


/* ----------------------------------------------------------------------------------
 * 설명 : 입력 문자열이 기계어 코드인지를 검사하는 함수이다. 
 * 매계 : 토큰 단위로 구분된 문자열 
 * 반환 : 정상종료 = 기계어 테이블 인덱스, 에러 < 0 
 * 주의 : 
 *		
 * ----------------------------------------------------------------------------------
 */
int search_opcode(char *str){
    //코드 작성
    char strtmp[7];
    strcpy(strtmp,str);

    //4형식이어서 +로 시작한다면 없애주기(* str을 바꾸면 안됨)
    if(strtmp[0] == '+')
        for(int j=0;j<strlen(strtmp);j++)
            strtmp[j]=strtmp[j+1];

    for(int i=0;i<inst_index;i++){
        if(!strcmp(inst_table[i]->mnemonic,strtmp))
            return i;
    }

    return -1;
}

/* ----------------------------------------------------------------------------------
* 설명 : 어셈블리 코드를 위한 패스1과정을 수행하는 함수이다.
*		   패스1에서는..
*		   1. 프로그램 소스를 스캔하여 해당하는 토큰단위로 분리하여 프로그램 라인별 토큰
*		   테이블을 생성한다.
*
* 매계 : 없음
* 반환 : 정상 종료 = 0 , 에러 = < 0
* 주의 : 현재 초기 버전에서는 에러에 대한 검사를 하지 않고 넘어간 상태이다.
*	  따라서 에러에 대한 검사 루틴을 추가해야 한다.
*
* -----------------------------------------------------------------------------------
*/
static int assem_pass1(void){
    /* input_data의 문자열을 한줄씩 입력 받아서 
	 * token_parsing()을 호출하여 token_unit에 저장
	 */
    int errno =0;
    static int symnum=0;

    //한 줄씩 매개로 넘겨줌
    for(int i=0;i<line_num;i++){
        if(token_parsing(input_data[i]) < 0){    
            errno=-1;
            break;
        }

        /*if(token_table[i]->label){
            symbol_unit[symnum]->symbol = token_table[i]->label;
            symbol_unit[symnum]->addr = token_table[i]->address;
            symnum++;
        }*/
    }

    return errno;
}

/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 명령어 옆에 OPCODE가 기록된 표(과제 4번) 이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
*        또한 과제 4번에서만 쓰이는 함수이므로 이후의 프로젝트에서는 사용되지 않는다.
* -----------------------------------------------------------------------------------
*/
void make_opcode_output(char *file_name){
    FILE *file;
    int opnum;
    char *tmpStr;

    //파일 이름이 없는경우->stdout에 출력
    //있는 경우->파일 이름으로 저장
    if(file_name == NULL){
        for(int i=0;i<token_line;i++){
            if(token_table[i]->label != NULL){
                printf("%s",token_table[i]->label);
            }
            printf("\t");
            tmpStr = strtok(token_table[i]->oper,"\r");
            printf("%s\t", tmpStr);

            //operand개수에 따라 다르게 처리
            if(token_table[i]->operand[0] != NULL){
                if(token_table[i]->operand[1] != NULL){
                    if(token_table[i]->operand[2] != NULL){
                        printf("%s",token_table[i]->operand[0]);
                        printf(",%s",token_table[i]->operand[1]);
                        tmpStr = strtok(token_table[i]->operand[2],"\r");
                        printf(",%s",tmpStr);
                    }
                    else{
                        printf("%s",token_table[i]->operand[0]);
                        tmpStr = strtok(token_table[i]->operand[1],"\r");
                        printf(",%s",tmpStr);
                    }
                }
                else{
                    tmpStr = strtok(token_table[i]->operand[0],"\r");
                    printf("%s",tmpStr);
                }
                printf("\t");
            }

            //해당 명령어에 해당하는 opcode출력
            if((opnum = search_opcode(token_table[i]->oper)) >= 0)
                printf("%s",inst_table[opnum]->opcode);

            printf("\n");
        }
    }
    else{
        file = fopen(file_name,"w");
        for(int i=0;i<token_line;i++){
            if(token_table[i]->label != NULL){
                fprintf(file,"%s",token_table[i]->label);
            }
            fprintf(file,"\t");
            tmpStr = strtok(token_table[i]->oper,"\r");
            fprintf(file,"%s\t", tmpStr);

            //operand개수에 따라 다르게 처리
            if(token_table[i]->operand[0] != NULL){
                if(token_table[i]->operand[1] != NULL){
                    if(token_table[i]->operand[2] != NULL){
                        fprintf(file,"%s",token_table[i]->operand[0]);
                        fprintf(file,",%s",token_table[i]->operand[1]);
                        tmpStr = strtok(token_table[i]->operand[2],"\r");
                        fprintf(file,",%s",tmpStr);
                    }
                    else{
                        fprintf(file,"%s",token_table[i]->operand[0]);
                        tmpStr = strtok(token_table[i]->operand[1],"\r");
                        fprintf(file,",%s",tmpStr);
                    }
                }
                else{
                    tmpStr = strtok(token_table[i]->operand[0],"\r");
                    fprintf(file,"%s",tmpStr);
                }
                fprintf(file,"\t");
            }

            //해당 명령어에 해당하는 opcode출력
            if((opnum = search_opcode(token_table[i]->oper)) >= 0)
                fprintf(file,"%s",inst_table[opnum]->opcode);

            fprintf(file,"\n");
        }
        fclose(file);
    }
}
/* ----------------------------------------------------------------------------------
* 설명 : malloc으로 메모리 할당한 것을 free해준다.
* 매계 : 없음
* 반환 : 없음
* 주의 :
* -----------------------------------------------------------------------------------
*/
void free_malloc(void){
    for(int j=0;j<sizeof(inst_table)/sizeof(inst *);j++){
        free(inst_table[j]->mnemonic);
        free(&inst_table[j]->format);
        free(inst_table[j]->opcode);
        free(&inst_table[j]->opernum);
        free(inst_table[j]);
    }
    for(int j=0;j<line_num;j++){
        free(input_data[j]);
    }
    for(int j=0;j<token_line;j++){
        free(token_table[j]->label);
        free(token_table[j]->oper);
        free(&token_table[j]->operand[0]);
        free(&token_table[j]->operand[1]);
        free(&token_table[j]->operand[2]);
        free(token_table[j]->operand);
        free(token_table[j]->comment);
        free(&token_table[j]->nixbpe);
        free(token_table[j]);
    }
}
/* ----------------------------------------------------------------------------------
* 설명 : symbol테이블을 16진수로 출력
* 매계 : 생성할 심볼테이블 파일명
* 반환 : 없음
* 주의 :
* -----------------------------------------------------------------------------------
*/
void make_symtab_output(char *file_name){

}

/* ----------------------------------------------------------------------------------
* 설명 : 어셈블리 코드를 기계어 코드로 바꾸기 위한 패스2 과정을 수행하는 함수이다.
*		   패스 2에서는 프로그램을 기계어로 바꾸는 작업은 라인 단위로 수행된다.
*		   다음과 같은 작업이 수행되어 진다.
*		   1. 실제로 해당 어셈블리 명령어를 기계어로 바꾸는 작업을 수행한다.
* 매계 : 없음
* 반환 : 정상종료 = 0, 에러발생 = < 0
* 주의 :
* -----------------------------------------------------------------------------------
*/
static int assem_pass2(void){
    int errno;

    
    return errno;
}

/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 object code (프로젝트 1번) 이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
*
* -----------------------------------------------------------------------------------
*/
void make_objectcode_output(char *file_name){
    if(file_name==NULL){

    }
    else{

    }
}