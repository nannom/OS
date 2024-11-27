void main();
extern "C" void _start() {
    main();
    return;
}
#define WIDTH 320
#define HEIGHT 200

//__asm__ __volatile__("mov %0, al;" :"=r"(keybuf) );
void Write_port_byte(unsigned short port, unsigned char data) {
    __asm__ __volatile__ (
        "mov dx, %0;"
        "mov al, %1;"
        "out dx, al;"
        :
        : "r" (port), "r" (data)
        : "dx", "al"
    );
}

unsigned char Read_port_byte(unsigned short port) {
    unsigned char KeyCode;

    __asm__ __volatile__ (
        "mov dx, %1;"
        "xor eax, eax;"
        "in al, dx;"
        "mov %0, al;"
        : "=r" (KeyCode)
        : "r" (port)
        : "eax", "edx"
    );

    return KeyCode;
}

typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
volatile char *video = (char*)(0xB8000);
volatile unsigned short position = 0;
volatile uint32_t _timer = 0;
int printf(const char* _text,...) {
    char* arg_ptr = (char*)(&_text) + sizeof(_text);
    for(int i = 0;_text[i] != '\0';i++) {
        if(_text[i] == '%') {
            i++;
            switch (_text[i]) {
                case 'd':
                {
                    int value = *((int*)arg_ptr);
                    arg_ptr += sizeof(int);

                    int temp = value;
                    int digits = 0;
                    if (temp <= 0) {
                        digits++;
                        temp = -temp;
                    }
                    while (temp != 0) {
                        digits++;
                        temp /= 10;
                    }
                    temp = value;
                    if (value < 0) {
                        *video++ = '-';
                        *video++ = 7;
                        temp = -temp;
                    }
                    for (int i = digits - 1; i >= 0; i--) {
                        char buffer = temp % 10 + '0';
                        temp /= 10;
                        *video++ = buffer;
                        *video++ = 7;
                        position++;
                    }
                    break;
                }
                case 's':
                {
                    char* value2 = *((char**)arg_ptr);
                    arg_ptr += sizeof(char*);
                    while(*value2 != '\0') {
                        if(*value2 == '\n') {
                            int d = (int)video - 0xB8000;
                            d -= d % 160;
                            d += 0xB8000 + 160;
                            video = (char*)d;
                            position += (short)80 - (short)(position % 80);
                        }
                        else {
                            *video++ = *value2;
                            *video++ = 7;
                            value2++;
                            position++;
                        }
                    }
                    break;
                }
                case 'c':
                {
                    char value3 = *((char*)arg_ptr);
                    arg_ptr += sizeof(char);
                    *video++ = value3;
                    *video++ = 7;
                    position++;
                    break;
                }
                default:
                    break;
            }
        }
        else {
            if(_text[i] == '\n') {
                int d = (int)video - 0xB8000;
                d -= d % 160;
                d += 0xB8000 + 160;
                video = (char*)d;
                position += (short)80 - (short)(position % 80);
            }
            else {
                *video++ = _text[i];
                *video++ = 7;
                position++;
            }
        }
    }
    Write_port_byte(0x3D4,0x0F);
    Write_port_byte(0x3D5,(unsigned char)(position & 0xFF));
    Write_port_byte(0x3D4,0x0E);
    Write_port_byte(0x3D5,(unsigned char)((position >> 8) & 0xFF));
    return 0;
}
char issame(const char* _text1,const char* _text2) {
    while(*_text1 != 0 && *_text2 != 0) {
        if(*_text1 != *_text2) {
            return 0;
        }
        _text1+=2;
        _text2++;
    }
    return 1;
}

volatile int kbd_state[256] = {
    0,
};
unsigned char transScan(unsigned char target)
{
	unsigned char result;

	switch (target) 
	{
    case 0x01: result = '\0'; break; //ESC
    case 0x02: result = '1'; break;
    case 0x03: result = '2'; break;
    case 0x04: result = '3'; break;
    case 0x05: result = '4'; break;
    case 0x06: result = '5'; break;
    case 0x07: result = '6'; break;
    case 0x08: result = '7'; break;
    case 0x09: result = '8'; break;
    case 0x0A: result = '9'; break;
    case 0x0B: result = '0'; break;
    case 0x0C: result = '-'; break;
    case 0x0D: result = '='; break;
	case 0x1E: result = 'a'; break;
	case 0x30: result = 'b'; break;
	case 0x2E: result = 'c'; break;
	case 0x20: result = 'd'; break;
	case 0x12: result = 'e'; break;
	case 0x21: result = 'f'; break;
	case 0x22: result = 'g'; break;
	case 0x23: result = 'h'; break;
	case 0x17: result = 'i'; break;
	case 0x24: result = 'j'; break;
	case 0x25: result = 'k'; break;
	case 0x26: result = 'l'; break;
	case 0x32: result = 'm'; break;
	case 0x31: result = 'n'; break;
	case 0x18: result = 'o'; break;
	case 0x19: result = 'p'; break;
	case 0x10: result = 'q'; break;
	case 0x13: result = 'r'; break;
	case 0x1F: result = 's'; break;
	case 0x14: result = 't'; break;
	case 0x16: result = 'u'; break;
	case 0x2F: result = 'v'; break;
	case 0x11: result = 'w'; break;
	case 0x2D: result = 'x'; break;
	case 0x15: result = 'y'; break;
	case 0x2C: result = 'z'; break;
	case 0x39: result = ' '; break; // 스페이스
    case 0x0E: result = 0x08; break; // 백스페이스 아스키코드 = 8
    case 0x29: result = '`'; break;
    case 0x1C: result = '\n'; break;
	default: result = 0xFF; break; 
		// 구현안된 것은 무시한다. 구분자는 0xFF
	}
	return result;
}
unsigned char TransScan(unsigned char target)
{
	unsigned char result;

	switch (target) 
	{
    case 0x01: result = '\0'; break; //ESC
    case 0x02: result = '!'; break;
    case 0x03: result = '@'; break;
    case 0x04: result = '#'; break;
    case 0x05: result = '$'; break;
    case 0x06: result = '%'; break;
    case 0x07: result = '^'; break;
    case 0x08: result = '&'; break;
    case 0x09: result = '*'; break;
    case 0x0A: result = '('; break;
    case 0x0B: result = ')'; break;
    case 0x0C: result = '_'; break;
    case 0x0D: result = '+'; break;
	case 0x1E: result = 'A'; break;
	case 0x30: result = 'B'; break;
	case 0x2E: result = 'C'; break;
	case 0x20: result = 'D'; break;
	case 0x12: result = 'E'; break;
	case 0x21: result = 'F'; break;
	case 0x22: result = 'G'; break;
	case 0x23: result = 'H'; break;
	case 0x17: result = 'I'; break;
	case 0x24: result = 'J'; break;
	case 0x25: result = 'K'; break;
	case 0x26: result = 'L'; break;
	case 0x32: result = 'M'; break;
	case 0x31: result = 'N'; break;
	case 0x18: result = 'O'; break;
	case 0x19: result = 'P'; break;
	case 0x10: result = 'Q'; break;
	case 0x13: result = 'R'; break;
	case 0x1F: result = 'S'; break;
	case 0x14: result = 'T'; break;
	case 0x16: result = 'U'; break;
	case 0x2F: result = 'V'; break;
	case 0x11: result = 'W'; break;
	case 0x2D: result = 'X'; break;
	case 0x15: result = 'Y'; break;
	case 0x2C: result = 'Z'; break;
	case 0x39: result = ' '; break; // 스페이스
    case 0x0E: result = 0x08; break; // 백스페이스 아스키코드 = 8
    case 0x29: result = '~'; break;
    case 0x1C: result = '\n'; break;
	default: result = 0xFF; break; 
		// 구현안된 것은 무시한다. 구분자는 0xFF
	}
	return result;
}
struct IDT {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_high;
} __attribute__((packed));

struct IDTR {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// IDT 테이블 및 IDTR 선언
struct IDT idt[256];
struct IDTR idtr;
char scan_code_table[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '};
void idt_ignore() {
    __asm__ __volatile__(
        "iretd;"
    );
}

void idt_timer() {
    __asm__ __volatile__(
        "cli;"
        "pusha;"
        "push ds;"
        "push es;"
        "push fs;"
        "push gs;"
        "mov ax, 0x08;" 
        "mov ds, ax;"
        "mov es, ax;"
        "mov fs, ax;"
        "mov gs, ax;"
    );
    _timer++;
    __asm__ __volatile__(
        "mov al, 0x20;"
        "out 0x20, al;"
        "pop gs;"
        "pop fs;"
        "pop es;"
        "pop ds;"
        "popa;"
        "sti;"
        "iretd;"
    );
}

void idt_keyboard() {
    __asm__ __volatile__(
        "cli;"                // 인터럽트 비활성화
        "pusha;"              // 모든 레지스터 저장
        "mov ax, 0x08;"       // 코드 세그먼트로 변경 (플랫폼에 따라 다를 수 있음)
        "mov ds, ax;"         // DS 변경
        "mov es, ax;"         // ES 변경
        "mov fs, ax;"         // FS 변경
        "mov gs, ax;"         // GS 변경
        "in al, 0x60;"        // 키보드 데이터 포트에서 데이터 읽기
    );
    printf("test");

    unsigned char keybuf;
    __asm__ __volatile__("mov %0, al;" : "=r"(keybuf));  // AL 레지스터에서 키 코드 저장
    printf("%c", scan_code_table[keybuf]);
    Write_port_byte(0x20, 0x20);
    __asm__ __volatile__(
        "mov al, 0x20;"           // PIC에 EOI 신호
        "out 0x20, al;"           // EOI 신호 전송
        "popa;"                   // 모든 레지스터 복원
        "sti;"                    // 인터럽트 활성화
        "iretd;"                  // 인터럽트 처리 종료, 이전 상태로 복귀
    );
}

void init_pic() {
    // ICW1: 초기화 명령어 워드 1
    Write_port_byte(0x20, 0x11);  // Master PIC
    Write_port_byte(0xA0, 0x11);  // Slave PIC

    // ICW2: 인터럽트 벡터 오프셋 설정
    Write_port_byte(0x21, 0x20);  // Master PIC: 0x20 (32)
    Write_port_byte(0xA1, 0x28);  // Slave PIC: 0x28 (40)

    // ICW3: Master-Slave 연결 설정
    Write_port_byte(0x21, 0x04);  // Master PIC: Slave PIC는 IRQ2에 연결
    Write_port_byte(0xA1, 0x02);  // Slave PIC: Slave ID는 2

    // ICW4: 추가 정보
    Write_port_byte(0x21, 0x01);  // Master PIC: 8086 모드
    Write_port_byte(0xA1, 0x01);  // Slave PIC: 8086 모드

    // 인터럽트 마스크 설정
    Write_port_byte(0x21, 0x00);  // 마스터 PIC의 모든 인터럽트를 허용
    Write_port_byte(0xA1, 0x00);  // 슬레이브 PIC의 모든 인터럽트를 허용
}

// PIT 초기화 함수
void init_pit() {
    uint32_t frequency = 10000; // 100Hz (10ms 간격)
    uint16_t divisor = 1193180 / frequency;
    
    // PIT 제어 레지스터 설정
    Write_port_byte(0x43, 0x36);  // Channel 0, Low/High byte access, Mode 3

    // Divisor 설정
    Write_port_byte(0x40, divisor & 0xFF);       // Low byte
    Write_port_byte(0x40, (divisor >> 8) & 0xFF); // High byte
}


// IDT 초기화 함수
void init_idt() {

    uint32_t ptr = (uint32_t)idt_ignore;
    for(int i = 0;i<256;i++) {
        if(i != 0x21) {
            idt[i].offset_low = ptr & 0xFFFF;
            idt[i].selector = 0x08;
            idt[i].zero = 0;
            idt[i].type_attr = 0x8E;
            idt[i].offset_high = (ptr >> 16) & 0xFFFF;
        }
    }
    // IDT 엔트리 설정
    //ptr = (uint32_t)idt_ignore;
    //idt[0].offset_low = ptr & 0xFFFF;
    //idt[0].selector = 0x08;
    //idt[0].zero = 0;
    //idt[0].type_attr = 0x8E;
    //idt[0].offset_high = (ptr >> 16) & 0xFFFF;

    //ptr = (uint32_t)idt_timer;
    //idt[0x20].offset_low = ptr & 0xFFFF;
    //idt[0x20].selector = 0x08;
    //idt[0x20].zero = 0;
    //idt[0x20].type_attr = 0x8E;
    //idt[0x20].offset_high = (ptr >> 16) & 0xFFFF;

    ptr = (uint32_t)idt_keyboard;
    idt[0x21].offset_low = ptr & 0xFFFF;
    idt[0x21].selector = 0x08;
    idt[0x21].zero = 0;
    idt[0x21].type_attr = 0x8E;
    idt[0x21].offset_high = (ptr >> 16) & 0xFFFF;
    idtr.base = (uint32_t)&idt;
    idtr.limit = sizeof(idt) - 1;
    // IDT 로드
    __asm__ __volatile__("lidt %0" : : "m"(idtr));
    __asm__ __volatile__("sti");
    printf("ready");
}
int cls() {
    for(char* v = (char*)(0xB8000);v<(char*)(0xBFFFF);v++) {
        *v = 0;
    }
    video = (char*)0xB8000;
    return 0;
}
int help() {
    printf("\n<help> get commands\n<cls> clear screen\n<WaSans> WaSans!!!\n");
    return 0;
}
int tetris() {
    cls();
    char blocks[20][10] = {
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0}
    };
    int x = 0,y = 0;
    int p_x = 3,p_y = 0;
    int timer = *((int*)0x30000) + 1000;
    while(1) {
        for(y = 0;y<20;y++) {
            *((char*)(0xB8018) + ((y + 4) * 160)) = ' ';
            *((char*)(0xB8018) + ((y + 4) * 160) + 1) = 0x70;
            *((char*)(0xB8018) + ((y + 4) * 160) + 2) = ' ';
            *((char*)(0xB8018) + ((y + 4) * 160) + 3) = 0x70;
            *((char*)(0xB8018) + ((y + 4) * 160) + 44) = ' ';
            *((char*)(0xB8018) + ((y + 4) * 160) + 45) = 0x70;
            *((char*)(0xB8018) + ((y + 4) * 160) + 46) = ' ';
            *((char*)(0xB8018) + ((y + 4) * 160) + 47) = 0x70;
        }
        for(x = 0;x<12;x++) {
            *((char*)(0xB8018) + (24 * 160) + (x * 4)) = ' ';
            *((char*)(0xB8018) + (24 * 160) + (x * 4) + 1) = 0x70;
            *((char*)(0xB8018) + (24 * 160) + (x * 4) + 2) = ' ';
            *((char*)(0xB8018) + (24 * 160) + (x * 4) + 3) = 0x70;
        }
        for(y = 0;y<20;y++) {
            for(x = 0;x<10;x++) {
                *((char*)(0xB801C) + ((y + 4) * 160) + (x * 4)) = ' ';
                *((char*)(0xB801C) + ((y + 4) * 160) + (x * 4) + 1) = blocks[y][x] * 0x10;
                *((char*)(0xB801C) + ((y + 4) * 160) + (x * 4) + 2) = ' ';
                *((char*)(0xB801C) + ((y + 4) * 160) + (x * 4) + 3) = blocks[y][x] * 0x10;
                if(y == p_y && x == p_x) {
                    *((char*)(0xB801C) + ((y + 4) * 160) + (x * 4)) = ' ';
                    *((char*)(0xB801C) + ((y + 4) * 160) + (x * 4) + 1) = 1 * 0x10;
                    *((char*)(0xB801C) + ((y + 4) * 160) + (x * 4) + 2) = ' ';
                    *((char*)(0xB801C) + ((y + 4) * 160) + (x * 4) + 3) = 1 * 0x10;
                }
            }
        }
            while((*((unsigned int*)0x30000)) < timer) {
            }
            timer += 1000;
            p_y += 1;
    }
    cls();
    return 0;
}

/*
0x00 quit
0x01 print
0x02 scan
0x03 if
0x04 iszero
0x05 islow
0x06 ishigh
0x07 plus
0x08 set
0x09 jmp
0xFF none
*/
void main() {
    //initPIC();
        const char hx[] = {
        '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
    };
    //unsigned char testcode[] = {0x02,0x1E,0x00,0x00,0x00,0x48,0x03,0x00,0x00,0x00,0x48,0x00,0x00,0x00,0x49,0x04,0x00,0x00,0x00,0x27,0x03,0x00,0x00,0x00,0x4A,0x00,0x00,0x00,0x49,0x04,0x00,0x00,0x00,0x35,0x09,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x4A,0x00,0x00,0x00,0x49,0x09,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x4A,0x00,0x00,0x00,0x4B,0x01,0x00,0x00,0x00,0x4C,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,'A'};
    //                          0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F    10   11   12   13   14   15   16   17   18   19   1A   1B   1C   1D   1E   1F   20   21   22   23   24   25   26   27   28   29   2A   2B   2C   2D   2E   2F   30   31   32   33   34   35   36   37   38   39   3A   3B   3C   3D   3E   3F   40   41   42   43   44   45   46   47   48   49   4A   4B   4C
    //                          0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37   38   39   40   41   42   43   44   45   46   47   48   49   50   51   52   53   54   55   56   57   58   59   60   61   62   63   64   65   66   67   68   69   70   71   72   73   74   75   76
    
    int i = 0;
    for(i = 0;i<256;i++) {
        kbd_state[i] = 0;
    }
    video = (char*)(0xB8000);
    _timer = 0;
    for(char* v = (char*)(0xB8000);v<(char*)(0xBFFFF);v++) {
        *v++ = 0;
        *v = 0;
    }
    printf("<help> to get help!\n>");
    init_pic();  // PIC 초기화
    //init_pit();  // PIT 초기화
    //init_idt();  // IDT 초기화
    *((char*)(0x20000)) = (char)0;
    //*((char*)(0x20001)) = (char)0;
    int time = 10000;
    while(1) {
        if(Read_port_byte(0x64) & 1) {
            //__asm__ __volatile__("int 0x21");
            char data = Read_port_byte(0x60);
            if(!(data & 0x80)) {
                printf("%c", scan_code_table[data]);
            }
        }
        for(i = 0;i<128 && 0;i++) {
            if(kbd_state[i] == 1) {
                if(transScan(i) == '\n') {
                    int d = (int)video - 0xB8000;
                    d -= d % 160;
                    d += 0xB8000;
                    if(issame(((char*)d)+2,"help") == 1) {
                        help();
                    }
                    else if(issame(((char*)d) + 2, "cls") == 1) {
                        cls();
                    }
                    else if(issame(((char*)d) + 2, "test") == 1) {
                        //int code_ = process.point;
                    }
                    else if(issame(((char*)d + 2), "tetris") == 1) {
                        tetris();
                    }
                    else {
                        printf("\nunknowing command");
                    }
                    printf("\n>");
                }
                else if(transScan(i) == '\b') {
                    if(video > (char*)0xB8000) {
                        video -= 2;
                        position--;
                        *video = 0;
                        if(*(video-2) == '\0') {
                            while(video > (char*)0xB8000) {
                                video -= 2;
                                position--;
                                *video = 0;
                                if(*(video - 2) != '\0') {
                                    break;
                                }
                            }
                        }
                    }
                    Write_port_byte(0x3D4,0x0F);
                    Write_port_byte(0x3D5,(unsigned char)(position & 0xFF));
                    Write_port_byte(0x3D4,0x0E);
                    Write_port_byte(0x3D5,(unsigned char)((position >> 8) & 0xFF));
                }
                else if(i == 42 || i == 54) {
                }
                else {
                    if(transScan(i) == 0xFF) {
                        //printf("%c", hx[i >> 4]);
                        //printf("%c", hx[i & 0xF]);
                    }
                    else {
                        if(kbd_state[42] == 0 && kbd_state[54] == 0) {
                            printf("%c", transScan(i));
                        }
                        else {
                            printf("%c", TransScan(i));
                        }
                    }
                }
                kbd_state[i] = 2;
            }
        }
        //printf("%d\n", _timer);
        //printf("abcde");
        if(video >= (char*)(0xB8FA0)) {
            video = (char*)(0xB8000);
        }
    }
}
