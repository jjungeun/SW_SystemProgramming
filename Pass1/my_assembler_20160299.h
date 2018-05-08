/*
 * my_assembler 함수를 위한 변수 선언 및 매크로를 담고 있는 헤더 파일이다.
 * 
 */
 #define MAX_INST 256
 #define MAX_LINES 5000
 #define MAX_OPERAND 3

 /*
  * instruction 목록 파일로부터 정보를 받아와서 생성하는 구조체 변수이다.
  * 구조는 각자의 instruction set의 양식에 맞춰 직접 구현하되 라인별로 하나의 instruction을 저장한다.
  */
  struct inst_unit{
      char mnemonic[6];
      int format;
      char opcode[2];
      int opernum;
  };
  typedef struct inst_unit inst;
  inst *inst_table[MAX_INST];
  int inst_index;

  /*
   * 어셈블리할 소스코드를 입력받는 테이블이다. 라인단위로 관리한다
   */
char *input_data[MAX_LINES];
static int line_num;
int label_num;

 /*
  * 어셈블리할 소스코드를 토큰단위로 관리하기 위한 구조체 변수
  * operator는 renaming을 허용한다.
  * nixbpe는 8bit 중 하위 6개의 bit를 이용하여 n,i,x,b,p,e를 표시한다.
  */
 struct token_unit{
     int address;
     char *label;
     char *oper;
     char *operand[MAX_OPERAND];
     char *comment;
     char nixbpe; //추후 프로젝트 사용
 };

 typedef struct token_unit token;
 token *token_table[MAX_LINES];
 static int token_line;

 /*
  *심볼을 관리하는 구조체이다.
  *심볼 테이블은 심볼 이름, 심볼 위치로 구성된다.
  */
 
 struct symbol_unit{
     char symbol[10];
     int addr;
 };

 typedef struct symbol_unit symbol;
 symbol sym_table[MAX_LINES];
 static int locctr;
 char *extRef[MAX_OPERAND];
 char *extDef[MAX_OPERAND];
//-----------------//



static char *input_file;
static char *output_file;
int init_my_assembler(void);
int init_inst_file(char *inst_file);
int init_input_file(char *input_file);
int token_parsing(char *str);
int search_opcode(char *str);
static int assem_pass1(void);
void make_opcode_output(char *file_name);
void free_malloc(void);

static int assem_pass2(void);
void make_objectcode_output(char *file_name);
void make_symtab_output(char *file_name);
