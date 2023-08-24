#include "tetris.h"
static struct sigaction act, oact;

int space;
long evalSize(tree* parent){
	int i = 0;
	int sum = 0;
	if(parent->level == VISIBLE_BLOCKS - 1) return 0;

	for (parent->child[i]; parent->child[i]; i++){
		sum += sizeof(struct _tree);
		sum += evalSize(parent->child[i]);
	}

	return sum;
}
int main(){
	//종료 조건 변수 exit은 0으로 초기화
	int exit=0;
	////////////시간 측정
	time_t start, stop;
	double duration;

	//라이버러리를 사용하기 위해 시작 조건
	initscr();
	//입력한 값이 출력되지 않게
	noecho();
	//keypad및 방향키 사용지원
	keypad(stdscr, TRUE);	
	//rand함수가 실행할 때마다 다르게 나올 수 있게 seed를 항상 변하는  시간으로 넘겨줌 
	srand((unsigned int)time(NULL));
		
	//rank 연결리스트 생성
	createRankList();
	//exit == 1이면 종료
	while(!exit){
		//다 지움
		clear();

		switch(menu()){
		//메뉴가 play면 게임 실행
		case MENU_PLAY: play(); break;
		//2주차 실습
		case MENU_RANK: rank();	break;
		//exit이면 종료
		//3주차 과제
		case MENU_REC_PLAY:start = time(NULL); recommendedPlay(); stop = time(NULL);break;
		//2주차 실습
		case MENU_EXIT: exit=1; break;
		default: break;
		}
	}
	//현존하는 연결리스트 기반 rank.txt 수정
	writeRankFile();
	duration = (double)difftime(stop, start);
	endwin();
	//다 지워줌
	system("clear");
	if(recommendFlag == 1){
		printf("The total memory for the tree : %lf\n", 1000000*(double)score/space);
		printf("The total time for recommended playing : %lf\n", score/duration);
	}	


	return 0;
}

void InitTetris(){
	int i,j;
	//field 0으로 전부 초기화
	for(j=0;j<HEIGHT;j++)
		for(i=0;i<WIDTH;i++)
			field[j][i]=0;
	//다음 블록 랜덤으로 생성
	for(i = 0; i < VISIBLE_BLOCKS; i++)nextBlock[0]=rand()%7;	
		//추가한 내용
	//두 번쨰 다음 블록의 ID를 임의로 생성

	//블록 회전수도 0으로 초기화
	blockRotate=0;
	//블록 위치 가운데 맨위에 지정
	blockY=-1;
	//블록 위치 가운데 맨위에 지정
	blockX=WIDTH/2-2;
	//점수 0으로 초기화
	score=0;	
	//게임 종료 옵션 0으로 초기화
	gameOver=0;
	//BlockDown에 사용되는 timed_out 초기화
	timed_out=0;
	//(필드, 블록 상자, 점수 상자 테두리) 생성

	DrawOutline();
	//필드 자체를 그림
	DrawField();
	//블록을 그림(블록의 위치, 블록의 위치, 그릴 블록, 그 블록 회전수, 블록을 채울 문자 모양)
/////////////////////////////////////////////////////////////////////////////////////////////////
	max = (tree*)malloc(sizeof(tree));
	max->accumulatedScore = 0;
	///이전 루트의 사용된 메모리를 측정해서 더한다.
	//root생성 및 초기화
	root = (tree*)malloc(sizeof(tree));
	root->accumulatedScore = score;
	for (int i = 0; i < HEIGHT; i++){
		for (int j = 0; j < WIDTH; j++){
			root->recField[i][j] = field[i][j];
		}
	}
	modified_recommend(root,0);		
/////////////////////////////////////////////////////////////////////////////////////////////////	
		//수정한 내용
	DrawBlockWithFeatures(blockY,blockX,nextBlock[0],blockRotate);
	
	//다음 블록을 그림
	DrawNextBlock(nextBlock);
	//점수를 그림
	PrintScore(score);
}

void DrawOutline(){	
	int i,j;
	/* 블럭이 떨어지는 공간의 태두리를 그린다.*/
	DrawBox(0,0,HEIGHT,WIDTH);

	/* next block을 보여주는 공간의 태두리를 그린다.*/
	//위에서 두칸 내려오고, 필드의 10칸 오른쪽에 생성
	move(2,WIDTH+10);
	//일단 screen에 제목 출력
	printw("NEXT BLOCK");
	//박스를 그린다
	//한칸 더 내리고 똑같은 x의 위치에 4*8의 크기로 박스 생성
	DrawBox(3,WIDTH+10,4,8);


		//수정한 내용
	//두번 째 다음 블록을 그릴  박스
	DrawBox(9, WIDTH+10,4,8);


	/* score를 보여주는 공간의 태두리를 그린다.*/
		//수정한 내용
	move(16,WIDTH+10);
	//제목 일단 출력
	printw("SCORE");
	//제목 바로  아래에 1*8 크기의 박스 생성
		//수정한 내용
	DrawBox(17,WIDTH+10,1,8);
}

int GetCommand(){
	int command;
	//명령 입력받기
	command = wgetch(stdscr);
	//명령어에 따른 처리
	switch(command){
	case KEY_UP:
		break;
	case KEY_DOWN:
		break;
	case KEY_LEFT:
		break;
	case KEY_RIGHT:
		break;
	case ' ':	/* space key*/
		/*fall block*/
		break;
	case 'q':
	case 'Q':
		command = QUIT;
		break;
	default:
		command = NOTHING;
		break;
	}
	return command;
}

int ProcessCommand(int command){
	int ret=1;
	int drawFlag=0;
	switch(command){
	//command가quit이면 종료
	case QUIT:
		ret = QUIT;
		break;
	//command가 KEY_UP이면 rotate
	case KEY_UP:
		if((drawFlag = CheckToMove(field,nextBlock[0],(blockRotate+1)%4,blockY,blockX)))
			blockRotate=(blockRotate+1)%4;
		break;
	//command가 KEY_DOWN이면 한칸 내리기
	case KEY_DOWN:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY+1,blockX)))
			blockY++;
		break;
	//command가 KEY_RIGHT이면 오른쪽으로 한칸
	case KEY_RIGHT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX+1)))
			blockX++;
		break;
	//command가 KEY_LEFT이면 왼쪽으로 한칸
	case KEY_LEFT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX-1)))
			blockX--;
		break;
	default:
		break;
	}
	//움직일 수 있다면 움직이기
	if(drawFlag){ 
		DrawChange(field,command,nextBlock[0],blockRotate,blockY,blockX);
	}
	return ret;	
}
//필드 그리기
void DrawField(){
	int i,j;
	for(j=0;j<HEIGHT;j++){
		//줄바꿈문자를를 사용할 경우 테두리가 부서짐
		//커서를 하나만 내림
		move(j+1,1);
		for(i=0;i<WIDTH;i++){
			//칸의 값이 1이면 색깔을 반전시켜 출력
			if(field[j][i]==1){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			//칸의 값이 0이면 비어있으니깐 .을 출력
			else printw(".");
		}
	}
}


void PrintScore(int score){
	//커서를 점수판에 위치시키고 점수를 출력
		//수정한 내용
	move(18,WIDTH+11);
	printw("%8d",score);
}

void DrawNextBlock(int *nextBlock){
	int i, j;

	for( i = 0; i < 4; i++ ){
		//커서를 nextblock을 그리는 박스로 이동
		move(4+i,WIDTH+13);
		for( j = 0; j < 4; j++ ){
			//1이면 reverse 특성으로 채운다.
			if( block[nextBlock[1]][0][i][j] == 1 ){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			//0이면 그냥 빈칸으로 채운다.
			else printw(" ");
		}
	}

	//추가한 내용
	//블록의 초기위치를 변경하여 두 번쨰 다음 블록을 그리는 과정
	for ( i = 0; i < BLOCK_HEIGHT; i++){		
		move(10+ i, WIDTH + 13);
		for ( j = 0; j < BLOCK_WIDTH; j++ ){
		//첫 번째 블록 아래에 두 번째 블록을 그린다.
			if( block[nextBlock[2]][0][i][j] == 1){
				//reverse 특성으로 채운다.
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////
//추가한 함수
void DrawBlockWithFeatures(int y, int x, int blockID, int blockRotate){
	//블록을 그린다.
	if(recommendFlag == 0) DrawBlock(y, x, blockID, blockRotate, ' ');
	//그림자를 그린다.
	if(recommendFlag == 0)	DrawShadow(y, x, blockID, blockRotate);
		
	DrawRecommend();
}

//////////////////////////////////////////////////////////////////////////////////
void DrawBlock(int y, int x, int blockID,int blockRotate,char tile){
	int i,j;
	for(i=0;i<4;i++)
		for(j=0;j<4;j++){
			//헤더파일을 통하여 주어진 argument에 따라 블록을 확인
			//해당 블록배열의 원소의 값이 1일 때 해당하는 위치 i+y+1, j+x+1을 색칠
			//아직 블록이 필드에 들어오지도 않았을 때는 출력하지 않는다.
			if(block[blockID][blockRotate][i][j]==1 && i+y>=0){
				move(i+y+1,j+x+1);
				attron(A_REVERSE);
				printw("%c",tile);
				attroff(A_REVERSE);
			}
		}
	
	move(HEIGHT,WIDTH+10);
}
void DrawBox(int y,int x, int height, int width){
	int i,j;
	//커서를 주어진 위치(y,x)로 이동
	move(y,x);
	//왼쪽 상단 모퉁이
	addch(ACS_ULCORNER);
	//윗변
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	//오른쪽 상단 모퉁이
	addch(ACS_URCORNER);
	//왼쪽변과, 오른쪽 변 같이 그리기
	for(j=0;j<height;j++){
		move(y+j+1,x);
		addch(ACS_VLINE);
		move(y+j+1,x+width+1);
		addch(ACS_VLINE);
	}
	//왼쪽 하단 모퉁이
	move(y+j+1,x);
	addch(ACS_LLCORNER);
	//아래변
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	//오른쪽 하단 모퉁이
	addch(ACS_LRCORNER);
}

void play(){
	int command;
	clear();
	//꾸준하게 한칸씩 내려준다.
	act.sa_handler = BlockDown;
	sigaction(SIGALRM,&act,&oact);
	//테트리스 초기화
	InitTetris();
	do{
		if(timed_out==0){
			alarm(1);
			timed_out=1;
		}
		//명령 받아오기
		command = GetCommand();
		//명령 수행하기
		//q나 Q눌렀을 때 종료하기
		if(ProcessCommand(command)==QUIT){
			alarm(0);
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
			move(HEIGHT/2,WIDTH/2-4);
			printw("Good-bye!!");
			refresh();
			getch();
			
			
			return;
		}
	}while(!gameOver);
	//게임 오버 될 경우 종료하기
	alarm(0);
	getch();
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
	move(HEIGHT/2,WIDTH/2-4);
	printw("GameOver!!");
	refresh();
	getch();
	newRank(score);
}
//메뉴 출력하기
//입력받은 메뉴 return하기
char menu(){
	printw("1. play\n");
	printw("2. rank\n");
	printw("3. recommended play\n");
	printw("4. exit\n");
	return wgetch(stdscr);
}

/////////////////////////첫주차 실습에서 구현해야 할 함수/////////////////////////

int CheckToMove(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	// user code
	// 블록의 원소 모두가 조건에 부합하는지 count할 때 사용할 변수
	int count = 0;
	// 블록을 위에서 아래로, 왼쪽에서 오른쪽으로 검사
	for (int i = 0; i < BLOCK_HEIGHT; i++){
		for (int j = 0; j < BLOCK_WIDTH; j++){
			//블록배열에서 채워져 있는 원소를  살핀다.
			if (block[currentBlock][blockRotate][i][j] == 1){
				//이미 블록이 쌓여있는 지 확인
				if(f[blockY + i][blockX + j] == 1){
					return 0;
				}
				//아랫변에 걸리는 경우
				if((blockY + i) >= HEIGHT){
					return 0;
				}
				//왼쪽변에 걸리는 경우
				if((blockX + j) < 0){
					return 0;
				}
				//오른쪽변에 걸리는 경우
				if((blockX + j) >= WIDTH){
					return 0;
				}
				
				//통과
				count++;
				
			}
		}
	}
	//블록 4개가 다 통과했다면 출력가능!
	if (count == 4 ){
		return 1;
	}
}

void DrawChange(char f[HEIGHT][WIDTH],int command,int currentBlock,int blockRotate, int blockY, int blockX){
	// user code
	//1. 이전 블록 정보를 찾는다. ProcessCommand의 switch문을 참조할 것
	//이전 블록의 y
	int preY = blockY;
	//이전 블록의 x
	int preX = blockX;
	//이전 블록의 회전 수
	int preR = blockRotate;
	switch(command){
		//command에 따라 이전 블록 정보를 찾아준다.
		//회전하기 이전의 회전 횟수 찾기
		case KEY_UP:
			preR = (preR + 3)%4;
		break;
		//한 칸 아래로 이동하기 전의 위치 찾기
		case KEY_DOWN:
			preY--;
		break;
		//한 칸 오른쪽으로 이동하기 전의 위치 찾기
		case KEY_RIGHT:
			preX--;
		break;
		//한 칸 왼쪽으로 이동하기 전의 위치 찾기
		case KEY_LEFT:
			preX++;
		break;

		default:
		break;
	}
	//2. 이전 블록 정보를 지운다. DrawBlock함수 참조할 것.
	for (int i = 0; i < BLOCK_HEIGHT; i++){
		for (int j = 0; j < BLOCK_WIDTH; j++){
			if(block[currentBlock][preR][i][j] == 1){
				//끝에 테두리를 생각해줘야 하기 때문에
				move(preY + i + 1, preX + j + 1);
				printw(".");
			}
		}
	}
	//이전 그림자 위치의 y좌표이다.
	//DrawShadow함수와 같은 방법으로 찾아준다.
	int preSY = preY;
	while(CheckToMove(f, currentBlock, preR, ++preSY, preX));
	preSY--;
	//이전 블록의 그림자도 함께 지운다.
	for (int i = 0; i < BLOCK_HEIGHT; i++){
		for(int j = 0;j < BLOCK_WIDTH; j++){
			if(block[currentBlock][preR][i][j] == 1){
				move(preSY + i + 1, preX + j + 1);
				printw(".");
			}
		}
	}

	//3. 새로운 블록 정보를 그린다.
	//블록을 다시 그릴 때는 DrawBlockWithFeatures() 함수를 이용한다.
	//블록의 위치와 그림자를 함께 그린다.
	DrawBlockWithFeatures(blockY, blockX, currentBlock, blockRotate);
}

void BlockDown(int sig){
	int i;
	// user code
	//옮길 수 있을 때
	if(CheckToMove(field,nextBlock[0],blockRotate,blockY+1,blockX) == 1){
		//바뀐 블록의 위치로 다시 그린다.
		DrawChange(field,KEY_DOWN, nextBlock[0],blockRotate, ++blockY,blockX);
	}
	//옮길 수 없을 때
	else {
		//천장에 닿은 경우
		//게임을 종료시킨다.
		if(blockY == -1){
			gameOver = 1;	
		}
	
		//바닥에 닿은  경우
		//필드에 쌓는 경우
		else{
		//완전한 line이 있을 경우 지워준다.
		//return 값들을 통해 score를 최신화해준다.
		score+=AddBlockToField(field,nextBlock[0], blockRotate, blockY, blockX)*10;
		score+=DeleteLine(field);
		//점수를 갱신한 뒤 출력한다.
		PrintScore(score);
		//Next 블록을 현재 블록으로 만들어주고
		for(i = 0; i < VISIBLE_BLOCKS-1; i++){
			nextBlock[i] = nextBlock[i+1];
		}
		nextBlock[i] = rand()%7;
		/////////////////////////////////
		
		free(max);
		max = (tree*)malloc(sizeof(tree));
		max->accumulatedScore = 0;
		//root생성 및 초기화
		root = (tree*)malloc(sizeof(tree));
		root->accumulatedScore = score;
		for (int i = 0; i < HEIGHT; i++){
			for (int j = 0; j < WIDTH; j++){
				root->recField[i][j] = field[i][j];
			}
		}
		modified_recommend(root, 0);	
		////////////////////////////////
		DrawNextBlock(nextBlock);
		//현재 블록의 위치를 초기화하고
		blockRotate = 0;
		blockY = -1;
		blockX = WIDTH/2 - 2;
		//동작을 종료한다.		
		DrawField();
		}
	}
	timed_out = 0;
	//강의자료 p26-27의 플로우차트를 참고한다.
}

int AddBlockToField(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	// user code
	//필드의 정보를 바꾼다.
	int touched = 0;
	for (int i = 0; i < BLOCK_HEIGHT; i++){
		for (int j = 0; j < BLOCK_WIDTH; j++){
			if(block[currentBlock][blockRotate][i][j] == 1){
				f[blockY + i][blockX + j] = 1;
					//추가한내용
				//바닥에 닿아있는 블록의 개수를 count한다.
				if ( f[blockY + i + 1][blockX + j] == 1 || blockY + i == HEIGHT - 1)touched++;
			}
		}
	}
	//Block이 추가된 영역의 필드값을 바꾼다.
	//바닥에 닿아있는 블록의 개수를 return한다.
	return touched;
}

int DeleteLine(char f[HEIGHT][WIDTH]){
	// user code

	//1. 필드를 탐색하여, 꽉 찬 구간이 있는지 탐색한다.
	//채워진 칸의 개수를 셀 변수이다.
	int filled_count = 0;
	//꽉 찬 라인의 인덱스를  저장할 배열이다.
	int line[4] = {-1,-1,-1,-1};
	//꽉 찬 라인의 개수이다.
	int k=0;
	//가장 밑에서부터 검사한다.
	for (int i = HEIGHT - 1; i >= 0; i--){
		//라인을 탐색할  때마다 변수를 초기화해준다.
		filled_count = 0;
		//라인의 왼쪽부터 탐색한다.
		for (int j = 0; j < WIDTH; j++){
			//빈칸이 아닐 경우 filled_count의 값을 하나씩 늘린다.
			if(f[i][j]){
				filled_count++;
			}
			//빈칸이 있을 경우 for문을 종료하고 다음 줄을 살핀다.
			else{
				j = WIDTH;
			}
			
		}
		//줄 전체가 꽉 찼을 경우, 배열에 해당 라인의 인덱스를 하나씩 저장한다. 
		if (filled_count == WIDTH){
			line[k] = i;
			k++;
			}
	}
	//2. 꽉 찬 구간이 있으면 해당 구간을 지운다. 즉, 해당 구간으로 필드값을 한칸씩 내린다.
	//2. 꽉 찬 구간이 있으면 해당 구간을 지운다. 즉, 해당 구간으로 필드값을 한칸씩 내린다.
	//꽉 찬 라인을 위에서부터 차례대로 지워준다.
	for(int i = k-1; i >= 0; i--){
		//지우는 줄의 윗쪽의 모든 줄을 아래에서부터 차례대로 한칸씩 내린다.
		for(int j = line[i]; j >= 1; j--){
			for(int l = 0; l < WIDTH; l++){
				f[j][l] = f[j-1][l];
			}

		}
	}
	return k*k*100;
}

///////////////////////////////////////////////////////////////////////////

void DrawShadow(int y, int x, int blockID,int blockRotate){
	// user code
	int shadowY = y;
	//while문을 통하여 끝까지 내려갔을 때의 위치를 찾는다.
	while(CheckToMove(field, blockID, blockRotate, ++shadowY,x));
	//while문을 탈출했을 때는 움직일 수 없는 자리였을 것이기  때문에
	//shadowY의 값을 1줄여준다.
	shadowY--;
	//shadowY는 가장 아래에 있다.
	DrawBlock(shadowY, x, blockID, blockRotate, '/');
}
void createRankList() {
	// 목적: Input파일인 "rank.txt"에서 랭킹 정보를 읽어들임, 읽어들인 정보로 랭킹 목록 생성
	// 1. "rank.txt"열기
	// 2. 파일에서 랭킹정보 읽어오기
	// 3. LinkedList로 저장
	// 4. 파일 닫기
	FILE* fp;
	int i, j;
	char name[NAMELEN+1];
	int score;
	//1. 파일 열기
	fp = fopen("rank.txt", "r");
	//파일이 없을 경우 파일생성
	// 2. 정보읽어오기
	/* int fscanf(FILE* stream, const char* format, ...);
	stream:데이터를 읽어올 스트림의 FILE 객체를 가리키는 파일포인터
	format: 형식지정자 등등
	변수의 주소: 포인터
	return: 성공할 경우, fscanf 함수는 읽어들인 데이터의 수를 리턴, 실패하면 EOF리턴 */
	// EOF(End Of File): 실제로 이 값은 -1을 나타냄, EOF가 나타날때까지 입력받아오는 if문
	if(fp){	
		fscanf(fp,"%d", &Nnode);
		for (i = 1; i<= Nnode; i++){ 
			fscanf(fp, "%s %d", name, &score);
			//temp노드를 생성해서
			Node* temp = (Node*)malloc(sizeof(Node));
			temp->score = score;
			strcpy(temp->name, name);
			temp->link = NULL;
			//첫번째일 경우 head와 tail을 생성
			if(i == 1){
				head = temp;
				tail = temp;
			}
			//head는 첫번째에 고정한 후 tail로 늘려나가면서 리스트 생성
			else{	
				tail->link = temp;
				tail = temp;
			}
		}	
	
	fclose(fp);
	}

}
void rank() {
	//목적: rank 메뉴를 출력하고 점수 순으로 X부터~Y까지 출력함
	//1. 문자열 초기화
	int x = 1, y = Nnode, ch, i, j;
	clear();

	//2. printw()로 3개의 메뉴출력
	printw("1. list ranks from X to Y\n");
	printw("2. list ranks by a specific name\n");
	printw("3. delete a specific rank\n");
	//3. wgetch()를 사용하여 변수 ch에 입력받은 메뉴번호 저장
	ch = wgetch(stdscr);

	//4. 각 메뉴에 따라 입력받을 값을 변수에 저장
	//4-1. 메뉴1: X, Y를 입력받고 적절한 input인지 확인 후(X<=Y), X와 Y사이의 rank 출력
	if (ch == '1') {
		//x와 y 받기
		echo();
		printw("X : ");
		scanw("%d", &x);
		
		printw("Y : ");
		scanw("%d", &y);
		noecho();
		//출력형식
		printw("       name        |     score      \n");
		printw("------------------------------------\n");
		j = 0;
		//j가 1일 때 출력하지 않음
		if(x > y || x>Nnode || Nnode==0 ){
			printw("search failure: no rank in the list\n");
			j = 1;
		}
		if(x == '\n'||x<1) x= 1;
		if(y == '\n'||y>Nnode) y = Nnode;
		Node* current = head;
		//j가 0일 때만 출력한다
		if(j == 0){
			//x번째까지 이동한다.
			for (i = 1; i < x; i++){
				current = current->link;
			}
			//y번째까지 출력한다.
			for (i = x; i <= y; i++){
				printw("%-19s|  %d\n", current->name, current->score);
				//tail에서는 더이상 이동할 수 없다.
				if(i != Nnode)current= current->link;
			}
		}
	}



	//4-2. 메뉴2: 문자열을 받아 저장된 이름과 비교하고 이름에 해당하는 리스트를 출력
	else if (ch == '2') {
		char str[NAMELEN + 1];
		//출력flag
		int check = 0;
		//일치하는 랭크를 찾을 때 사용
		Node* current = head; 
		Node* previous = NULL;
		//현재 확인 중인 랭크의 순위
		int i = 1;
		//입력하는 값이 출력되게 하기 위함
		echo();
		printw("input the name: ");
		scanw("%s", str);	
		noecho();
		
		//출력형식
		printw("       name        |     score      \n");
		printw("------------------------------------\n");
		//기존 랭크가 존재할 때만 검색
		if(Nnode){
			//탐색은 마지막 랭크까지만 진행한다.
			while(i <= Nnode){
				//비교결과 같을 경우 해당 랭크를 출력한다.
				if(strcmp(current->name, str) == 0){
					printw("%-19s|  %d\n",current->name, current->score);
					check = 1;
				}
				//마지막랭크일 경우 다음 랭크로 이동할 수가 없다.
				if(i < Nnode){
					previous = current;
					current = current->link;
				}
				i++;
				
			}
		}
		//check가 0일 경우 한 번도 같은 이름의 랭크를 찾지 못했다는 것을 의미한다.
		if(check == 0)printw("search failure: no name in the list\n");
	
	}

	//4-3. 메뉴3: rank번호를 입력받아 리스트에서 삭제
	else if (ch == '3') {
		int num;
		echo();
		printw("input the rank: ");
		scanw("%d", &num);
		noecho();
		//삭제할 랭크를 탐색할 때 사용한다.
		Node* current = head;
		Node* previous = NULL;
		//삭제할 랭크를 삭제할 때 사용한다.
		Node* delete = NULL;
		int i;
		//리스트가 존재하고 정상적인 num일 때만 탐색하여 삭제한다.
		if(Nnode && num>=1 && num <= Nnode){
			//첫번째 랭크를 삭제할 경우
			//head노드이기 때문에 따로 처리한다.
			if(num == 1){
				delete = head;
				head = head->link;
			}
			else{
				//삭제할 랭크를 탐색한다.
				for (i = 1 ;  i < num; i++){
					previous = current;
					current = current->link;
				}
				//마지막 랭크를 삭제할 경우
				//tail노드이기 때문에 따로 처리한다.
				if(i == Nnode){
					delete = tail;
					tail = previous;
				}
				else{
					delete = current;
					previous->link = current->link;
				}
			}
			//delete를 메모리 해제하고 리스트의 개수를 1 줄여준다.
			
			free(delete);
			printw("\nresult: the rank deleted\n");
			Nnode--;
		}
		//리스트가 없거나 정상적인 num이 아닐 경우 메시지를 출력한다.
		else printw("search failure: no rank in the list\n");
	}
	getch();

}

void writeRankFile() {
	// 목적: 추가된 랭킹 정보가 있으면 새로운 정보를 "rank.txt"에 쓰고 없으면 종료
	int i;
	//1. "rank.txt" 연다
	FILE* fp = fopen("rank.txt", "w");
	//2. 랭킹 정보들의 수를 "rank.txt"에 기록
	//3. 탐색할 노드가 더 있는지 체크하고 있으면 다음 노드로 이동, 없으면 종료
	Node* current = head;
	fprintf(fp, "%d\n", Nnode);
	for (i = 1; i <= Nnode ; i++){
		fprintf(fp, "%s %d\n", current->name, current->score);
		if(i < Nnode)current = current->link;
	}

	current = head;
	Node* del;
	for (i = 1; i <= Nnode; i++) {
		del = current;
		if(i < Nnode)current = current->link;
		free(del);
	}
}
	// 목적: GameOver시 호출되어 사용자 이름을 입력받고 score와 함께 리스트의 적절한 위치에 저장
void newRank(int score){
	char str[NAMELEN + 1];
	int i, j;

	clear();
	//1. 사용자 이름을 입력받음
	printw("your name : ");
	echo();
	scanw("%s", str);
	noecho();
	//새로운 랭크의 위치 찾기를 위함
	Node* current = head;
	Node* previous = NULL;
	
	//2. 새로운 노드를 생성해 이름과 점수를 저장, score_number가
	Node *temp = (Node*)malloc(sizeof(Node));
	strcpy(temp->name, str);
	temp->score = score;
	temp->link = NULL;
	//첫 랭크
	if (Nnode == 0) {
		head = temp;
		tail = temp;
	}
	//추가
	else{
		//current의 노드 번째
		i = 1;
		while(current->score > score && i < Nnode ){
			previous = current; current = current->link;
			i++;
		}
		//마지막번째
		if( i == Nnode && current->score >= score ){
			tail -> link = temp;
			tail = temp;
		}
		//가장 높은 점수
		else if ( i == 1){
			temp -> link = head;
			head = temp;
		}
		//중간
		else{
			previous->link = temp;
			temp->link = current;
		}
	}
	Nnode++;

}
void DrawRecommend(){
	//첫번째블록이 어디에 놓여질지 찾는다.
	tree* temp = max;
	while(temp->level != 1)temp = temp->parent;
	DrawBlock(temp->recBlockY,temp->recBlockX,temp->curBlockID,temp->recBlockRotate, 'R');
}

int modified_recommend(tree* root, int bi){
	//child tree의 index bi
	int childN = 0;
	if( bi < VISIBLE_BLOCKS){
		//For each rotation of block
		for(int recRotate = 0; recRotate < blockrotate(nextBlock[bi]); recRotate++){
			int recX = 0; int recY = 0;
			//recX를 가장 왼쪽으로 set
			while(CheckToMove(root->recField, nextBlock[bi], recRotate, recY, --recX)); ++recX;
			
			//For each position possible to put the block
			for ( ; CheckToMove(root->recField, nextBlock[bi], recRotate, recY, recX); recX++){
				//recY를 가장 아래로 set
				while(CheckToMove(root->recField, nextBlock[bi], recRotate, ++recY, recX)); --recY;
				//가능한 경우(recY, recX) 설정 완료
				//child메모리 할당
				root->child[childN] = (tree*)malloc(sizeof(tree));
				if(root->child[childN] == NULL)exit(1);
				//store all the information
				///////////////////////////copy//////////////////////////////
				for (int i = 0 ; i < HEIGHT; i++){
					for (int j = 0; j < WIDTH; j++){
						root->child[childN]->recField[i][j] = root->recField[i][j];
					}
				}
				//////////////////////////////////////////////////////////////
								//점수 초기화
				root->child[childN]->accumulatedScore = root->accumulatedScore;
				//Make a new field where block is put
				//////////////////////////////Add//////////////////////////////////////////
				int touched = 0;
				for (int i = 0; i < BLOCK_HEIGHT; i++){
					for (int j = 0; j < BLOCK_WIDTH; j++){
						if(block[nextBlock[bi]][recRotate][i][j] == 1){
							root->child[childN]->recField[recY + i][recX + j] = 1;
							if ( root->child[childN]->recField[recY+i+1][recX + j]  || recY + i == HEIGHT - 1)touched++;
						}
					}
				}
				root->child[childN]->accumulatedScore += 10 * touched;
				//////////////////////////////////////////////////////////////////////////
				
				////////////////////////////////DELETE/////////////////////////////////////
				int filled_count = 0;
				int line[4] = {-1,-1,-1,-1};
				int k=0;
				for (int i = HEIGHT - 1; i >= 0; i--){
					filled_count = 0;
					for (int j = 0; j < WIDTH; j++){
						if(root->child[childN]->recField[i][j]){
							filled_count++;
						}
						else{
							break;
						}
					}
					if (filled_count == WIDTH){
						line[k] = i;
						k++;
					}
				}
				//지우는 줄의 윗쪽의 모든 줄을 아래에서부터 차례대로 한칸씩 내린다.
				for(int i = k-1; i >= 0; i--){
					for(int j = line[i]; j >= 1; j--){
						for(int l = 0; l < WIDTH; l++){
							root->child[childN]->recField[j][l] = root->child[childN]->recField[j-1][l];
						}
					}
				}
				root->child[childN]->accumulatedScore +=  (k*k*100);
				////////////////////////////////////////////////////////////////////////////
					
				//빈 구멍을 만드는 것을 방지하기 위함이다. 
				//블록을 이왕이면 낮은 곳에 쌓기 위함이다.
				///////////////////////////////MODIFIED////////////////////////////////////
				//빈칸을 센다
				int empty = 0;
				for(int i = 0; i < WIDTH; i++){
					//X별로  가장 높이 있는 블록을 찾는다.
					int top;
					for(top = 0; !root->child[childN]->recField[top][i] && top < HEIGHT; top++);
					//이왕이면 낮은 곳에 쌓을 수 있도록 점수를 깎았다.
					if(top < HEIGHT / 2) root->child[childN]->accumulatedScore -= 10;

					//가장 높이 있는 블록 아래에 있는 빈 칸을 센다.
					for(int j = top; j < HEIGHT; j++){
						if(root->child[childN]->recField[j][i] == 0) empty++;
					}
				}
				//구멍을 만드는 것을 피하기 위해 점수를 깎았다.
				root->child[childN]->accumulatedScore -= 10 * empty;

				////////////////////////////////////////////////////////////////////////////
					//parent 기억
					root->child[childN]->parent = root;
					//level 기억
					root->child[childN]->level = bi+1;

					//해당 블록 ID 기억
					root->child[childN]->curBlockID = nextBlock[bi];
					//해당 X값 기억
					root->child[childN]->recBlockX = recX;
					//해당 Y값 기억
					root->child[childN]->recBlockY = recY;
					//해당 회전수 기억
					root->child[childN]->recBlockRotate = recRotate;
						
					//재귀
					modified_recommend(root->child[childN], bi+1);
					childN++;
			}
		}
	}
	//update the information of the block when the accumulated maximum score < score
	else{
		if(root->accumulatedScore > max->accumulatedScore){
			max = root;
		}
		
	}
	//tree제작 완료
	//maximum case 
	return max->accumulatedScore;

}
void freeTree(tree* child){
	if(child->parent->level != 0){
		freeTree(child->parent);
		free(child);
	}
}


void rec_BlockDown(int sig){
	//점수가 가장 큰 경우의 첫 번째 블록을 찾는다.	
	tree* temp = max;
	while(temp->level != 1)temp = temp->parent;
	
	//블록이 끝까지 찼을 경우 종료한다.
	if( temp->recBlockY  <= 1 ) gameOver = 1;
	else{

		///////////////////////점수 최신화///////////////////////////////////////////
		
		///////////////////////필드 최신화////////////////////////////////////////////
		score += 10 * AddBlockToField(field, nextBlock[0], temp->recBlockRotate, temp->recBlockY, temp->recBlockX);
		score += DeleteLine(field);
		PrintScore(score);
		////////////////////다음 블록들을 생성한다.////////////////////////////////////
		for(int i = 0; i < VISIBLE_BLOCKS-1; i++)nextBlock[i] = nextBlock[i+1];
		nextBlock[VISIBLE_BLOCKS-1] = rand()%7;
		//블록 위치 초기화
		blockRotate = 0;
		blockY = -1;
		blockX = WIDTH/2 - 2;
		
		//가장 점수가 큰 경우를 가리키는 max를 생성한다.
		free(max);
		max = (tree*)malloc(sizeof(tree));
		max->accumulatedScore = 0;
		//모든 경우를 저장할 root를 생성한다.
		space += evalSize(root);
		root = (tree*)malloc(sizeof(tree));
		root->accumulatedScore = score;
		root->level = 0;
		//root의 field를 초기화 한다.
		for (int i = 0; i < HEIGHT; i++){
			for (int j = 0; j < WIDTH; j++){
				root->recField[i][j] = field[i][j];
			}
		}
		//가장 점수가 큰 경우를 찾는다.
		modified_recommend(root, 0);	
		////////////////////////////////
		DrawNextBlock(nextBlock);
		//현재 블록의 위치를 초기화하고
		blockRotate = 0;
		blockY = -1;
		blockX = WIDTH/2 - 2;
		//동작을 종료한다.		
		DrawField();
		}
	//블록들을 그린다.
	DrawBlockWithFeatures(blockY, blockX, nextBlock[0], blockRotate);
	timed_out = 0;
}


void recommendedPlay(){
	//recommendFlag가 1일 때는 recommendedPlay에 맞춰 실행한다.
	recommendFlag = 1;
	int command;
	
	clear();
	//꾸준하게 한칸씩 내려준다.
	//게임을 진행한다.
	act.sa_handler = rec_BlockDown;
	sigaction(SIGALRM,&act,&oact);
	//테트리스 초기화
	InitTetris();
	do{
		if(timed_out==0){
			alarm(1);
			timed_out=1;
		}
		//명령 받아오기
		command = GetCommand();
		//수행할 수 있는 command는 오직 종료(Quit)이다.
		if (command != QUIT)command = NOTHING;
		//명령 수행하기
		//q나 Q눌렀을 때 종료하기
		if(ProcessCommand(command)==QUIT){
			alarm(0);
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
			move(HEIGHT/2,WIDTH/2-4);
			printw("Good-bye!!");
			refresh();
			getch();
			
			
			return;
		}
	}while(!gameOver);
	//게임 오버 될 경우 종료하기
	alarm(0);
	getch();
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
	move(HEIGHT/2,WIDTH/2-4);
	printw("GameOver!!");
	refresh();
	getch();
}
//블록별로 가능한 회전수를 return한다.
int blockrotate(int blockID){
	switch(blockID){
		case 0 : return 2; break;
		case 1 : return 4; break;
		case 2 : return 4; break;
		case 3 : return 4; break;
		case 4 : return 1; break;
		case 5 : return 2; break;
		case 6 : return 2; break;
		default :  break;
	}
}

