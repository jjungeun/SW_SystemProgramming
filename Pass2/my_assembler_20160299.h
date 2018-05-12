/*
 * my_assembler 함수를 위한 변수 선언 및 매크로를 담고 있는 헤더 파일이다.
 * 
 */
 #define MAX_INST 256
 #define MAX_LINES 5000
 #define MAX_OPERAND 3
 #define NBIT 32
 #define IBIT 16
 #define XBIT 8
 #define BBIT 4
 #define PBIT 2
 #define EBIT 1	
 #define FFF 4095		// PC relative에서 TA < PC인 경우 보수계산을 위해

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
     int address;				 //locctr값을 같이 저장한다.
     char *label;
     char *oper;
     char *operand[MAX_OPERAND];
     char *comment;
     char nixbpe;
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
     int section;   //해당 symbol이 어느 section에 속했는지 알기 위해서
 };

 typedef struct symbol_unit symbol;
 symbol sym_table[MAX_LINES];
 static int locctr;
 static int symnum;
 static int section;
 int section_length[10];			//section의 길이를 저장하여 objectprogram을 만들때 사용한다.
 int retadr=0;						//RETADR을 위해 사용한다.
 
 struct literal_unit{
     char name[10]; 
     char value[10];    //literal 값
     int length;    //literal 길이
     int addr;
     int section;   //해당 literal이 어느 section에 속했는지 알기 위해
     char type; //char인지 byte인지 알기 위해
 };
 typedef struct literal_unit literal;
 literal *literal_table[MAX_LINES];
 static int literalnum;

 char objectcode[MAX_LINES];

 //Modification record를 위한 구조체
 struct reference_unit{
     char name[10];
     int addr;  
     int section;
     int size;          //몇비트를 고쳐야하는지 알기 위해
     char sign;         //+인지 -인지 알기 위해
 };
 typedef struct reference_unit refer;
 refer ref_table[10];
 int refnum;
 //-----------------//

 static char *input_file;
 static char *output_file;
 int init_my_assembler(void);
 int init_inst_file(char *inst_file);
 int init_input_file(char *input_file);
 int token_parsing(char *str);
 int search_opcode(char *str);
 int search_symbol(char *str);
 static int assem_pass1(void);
 void make_opcode_output(char *file_name);
 void free_malloc(void);

 static int assem_pass2(void);
 void make_objectcode_output(char *file_name);
 void make_symtab_output(char *file_name);
