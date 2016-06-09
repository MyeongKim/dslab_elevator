#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <poll.h>
#include <pthread.h>

struct input{
    int floor;
    char direction[10];
};

struct node{
    struct input element;
    struct node* next;
};

struct elevator{
    int number;
    int current_floor;
    bool moving;
    struct node* start;
    int people;
    bool is_broken;
};

struct elevator elevator_array[3] = {
    {1, 1, false, NULL, 0, false},
    {2, 1, false, NULL, 0, false},
    {3, 1, false, NULL, 0, false}
};

char notice[100];
int pause_status=0;
int button_status[10][2]={
    {-1,0},
    {0,0},
    {0,0},
    {0,0},
    {0,0},
    {0,0},
    {0,0},
    {0,0},
    {0,0},
    {0,-1}
};

int pass_floor_array[3][10]={
    {1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1}
};

void random_floor_select(int, int, char[]);

void *elevator_move(void *arg)
{
    struct node *curr;
    int destination;
    int *current_floor;
    int number = *(int *)arg;
    int broken_type;
    
    srand(time(NULL));
    while(1)
    {
        sleep(1); // 1초 동안 이동
        curr = elevator_array[number-1].start;
        if(curr)
        {
            destination = curr->element.floor;
            elevator_array[number-1].moving = true;
            current_floor = &elevator_array[number-1].current_floor;
            while((*current_floor != destination) && (elevator_array[number-1].start != NULL))
            {
                if(pause_status != 1) // 일시정지가 아닐 경우
                {
                    if(elevator_array[number-1].is_broken == false)
                    {
                        sleep(1);
                        if(*current_floor > destination)
                        {
                            *current_floor = *current_floor - 1;
                        }
                        else
                        {
                            *current_floor = *current_floor + 1;
                        }
                    }
                    else
                    {
                        // 만약 이 엘리베이터가 고장난 상태일 경우
                        broken_type = rand()%2;
                        if((broken_type == 0) && (*current_floor != 1))
                        {
                            // 일정 시간 동안 작동 하지 않는 고장
                            elevator_array[number-1].is_broken = false;
                            sleep(5);
                        }
                        else
                        {
                            // 1층으로 떨어지는 고장
                            *current_floor = 1;
                        }
                    }
                }
            }
            
            // 엘리베이터가 목적지에 도착
            if(elevator_array[number-1].start != NULL)
            {
                // 다음 층 노드가 있으면 다음 층으로 출발
                elevator_array[number-1].start = curr->next;
                
                // 버튼 상태 배열 재설정
                if(strcmp(curr->element.direction, "아래") == 0)
                {
                    if(destination != 1)
                        button_status[destination-1][0] = 0;
                }
                else
                {
                    if(destination != 10)
                        button_status[destination-1][1] = 0;
                }
                
                // 사람이 탔을때
                if(elevator_array[number-1].people != 0)
                {
                    // 랜덤 층 설정
                    elevator_array[number-1].people--;
                    random_floor_select(number, *current_floor, curr->element.direction);
                }
            }
            
        }
        elevator_array[number-1].moving = false;
        
    }
    return NULL;
}

void elevator_add_floor(int number, int floor, char direction[])
{
    struct node *curr, *temp;
    struct input new_input;
    int temp_num=-1;
    
    curr = elevator_array[number-1].start;
    temp = NULL;
    
    // 새로운 노드를 링크드 리스트의 특정한 위치에 삽입한다.
    while(1)
    {
        if(curr)
        {
            if(strcmp(direction, "위") == 0)
            {
                // 현재 엘리베이터가 위로 가고 있을 때
                if(temp_num == -1)
                    temp_num = 0;
                
                // 그 층으로 가는 길일 경우 중간에 삽입
                if((temp_num <= floor) && (floor <= curr->element.floor))
                {
                    break;
                }
            }
            else
            {
                // 현재 엘리베이터가 아래로 가고 있을 때
                if(temp_num == -1)
                    temp_num = 99;
                
                // 그 층으로 가는 길일 경우 중간에 삽입
                if((temp_num >= floor) && (floor >= curr->element.floor))
                {
                    break;
                }
            }
            
            temp = curr;
            temp_num = curr->element.floor;
            curr = curr->next;
        }
        else
            break;
    }
    
    // 새로운 층 노드 생성
    curr = (struct node*)malloc(sizeof(struct node));
    new_input.floor = floor;
    strcpy(new_input.direction, direction);
    curr->element = new_input;
    curr->next = NULL;
    
    // 바로 전 노드에 새로운 노드 연결
    if(temp)
    {
        temp->next = curr;
    }else{
        elevator_array[number-1].start = curr;
    }
    return;
}

void elevator_remove_node(int floor, char direction[])
{
    // 3개의 엘리베이터중 특정 노드를 검색하고 삭제
    int i;
    struct node *curr, *prev;
    for(i=0; i<3; i++)
    {
        curr = elevator_array[i].start;
        prev = elevator_array[i].start;

        while((curr != NULL) && (curr->element.floor != floor))
        {
            prev = curr;
            curr = curr->next;
        }
        
        if((curr != NULL) && (curr->element.floor == floor))
        {
            if(prev != elevator_array[i].start)
            {
                prev->next = curr->next;
            }
            else
            {
                elevator_array[i].start = curr->next;
            }
            elevator_array[i].people--;
        }
    }
    return;
}

void random_floor_select(int number, int destination, char direction[])
{
    // 랜덤한 층 지정
    int rand_num;
    int rand_dirct;
    srand(time(NULL));
    
    while(1)
    {
        
        rand_num = rand()%10+1;
        rand_dirct = rand()%2;
        if(pass_floor_array[number-1][rand_num-1] != 0)
        {
            if(strcmp(direction, "위") == 0)
            {
                if(rand_num > destination )
                    break;
            }else{
                if(rand_num < destination)
                    break;
            }
        }
    }
    sprintf(notice, "랜덤으로 선택된 층 : [%d]층\n", rand_num);
    elevator_add_floor(number, rand_num, "아래");
    return;
}

void push_button(int floor, char direction[])
{
    int selected_elevator_number;
    int i, temp, min_length=100;
    struct node *curr;
    
    // 3개 중 어떤 엘리베이터가 선택되어야 하는가?
    for(i=0; i<3; i++)
    {
        if((elevator_array[i].is_broken == false) && (pass_floor_array[i][floor-1] != 0))
        {
            // 엘리베이터가 고장난 상태가 아니면서 통과하는 층이 아닌 경우
            curr = elevator_array[i].start;
            if(curr != NULL)
            {
                // 엘리베이터가 움직이고 있을 때 도착하기까지 거리를 계산
                if(elevator_array[i].current_floor <= floor && floor <= elevator_array[i].start->element.floor)
                {
                    temp = abs(floor - elevator_array[i].current_floor);
                }
                else if(elevator_array[i].start->element.floor < floor)
                {
                    temp = abs(floor - elevator_array[i].current_floor);
                }
                else
                {
                    temp = abs(elevator_array[i].start->element.floor - floor);
                }
            }else{
                // 엘리베이터가 움직이고 있지 않을 때 도착하기까지 거리를 계산
                temp = abs(floor - elevator_array[i].current_floor);
            }
            
            if(temp < min_length)
            {
                // 3개의 엘리베이터 중 최소 거리를 가진 엘리베이터를 선택
                min_length = temp;
                selected_elevator_number = i+1;
            }
        }
    }
    
    if(min_length != 100)
    {
        // 3개 중 하나가 선택 되었을 때
        elevator_array[selected_elevator_number-1].people++;
        elevator_add_floor(selected_elevator_number, floor, direction);
    }
    else if((pass_floor_array[0][floor-1] == 0) && (pass_floor_array[1][floor-1] == 0) && (pass_floor_array[2][floor-1] == 0))
    {
        // 3개 모두 통과 층일때
        sprintf(notice, "이 층에 도달할 수 있는 엘리베이터가 없습니다. \n");
        if(strcmp(direction, "위") == 0)
            button_status[floor-1][1] = 0;
        else
            button_status[floor-1][0] = 0;
    }
    else
    {
        sprintf(notice, "모든 엘리베이터가 고장난 상태입니다. \n");
    }
    
    return;
}

void button_manager(void)
{
    int floor=0;
    int i;
    char direction[10];
    
    printf("층을 입력하세요. (돌아가기 : -1):  ");
    scanf("%d", &floor);
    
    if(floor == -1)
        return;
    
    printf("방향을 입력하세요(위/아래). (돌아가기 : -1):  ");
    scanf("%s", direction);
    
    system("clear");
    
    if(strcmp(direction, "-1") == 0)
        return;
    else if(strcmp(direction, "위")==0 && floor == 10)
    {
        sprintf(notice, "10층은 [아래] 버튼만 선택할 수 있습니다. \n");
        return;
    }
    else if(strcmp(direction, "아래")==0 && floor == 1)
    {
        sprintf(notice, "1층은 [위] 버튼만 선택할 수 있습니다. \n");
        return;
    }
    else if (strcmp(direction, "위") != 0 && strcmp(direction, "아래") != 0)
    {
        sprintf(notice, "방향 입력이 잘못되었습니다. 다시 입력하세요.\n");
        return;
    }

    // 이미 눌린 버튼인지 검사하고 취소
    if((button_status[floor-1][0] == 1) && (strcmp(direction, "아래") == 0))
    {
        button_status[floor-1][0] = 0;
        elevator_remove_node(floor, direction);
        return;
    }
    if((button_status[floor-1][1] == 1) && (strcmp(direction, "위") == 0))
    {
        button_status[floor-1][1] = 0;
        elevator_remove_node(floor, direction);
        return;
    }
    
    sprintf(notice, ">> %d층 %s 방향 버튼을 눌렀습니다. <<\n", floor, direction);
    if(strcmp(direction, "위") == 0)
        button_status[floor-1][1] = 1;
    else
        button_status[floor-1][0] = 1;
    push_button(floor, direction);
    
    return;

}

void simulation_pause(void)
{
    pause_status=1;
    sprintf(notice, "시뮬레이션이 일시정지되었습니다. \n");
    return;
}

void simulation_start(void)
{
    pause_status=0;
    sprintf(notice, "시뮬레이션이 재시작되었습니다. \n");
    return;
}


void simulation_manager(void)
{
    int selected;
    
    printf("=================================\n");
    printf("[1] 시뮬레이션 일시정지     [2] 시뮬레이션 시작\n");
    printf("=================================\n");
    
    printf("메뉴를 선택하십시오(종료 : -1):  ");
    scanf("%d", &selected);
    
    system("clear");
    switch(selected){
        case 1 : simulation_pause(); break;
        case 2 : simulation_start() ; break;
        case -1 : return;
        default : printf("잘못된 입력입니다. \n"); break;
    }
    
    return;

}

void break_elevator()
{
    // 3개 중 고장나지 않은 엘리베이터를 선택해서 랜덤으로 고장내기
    int rand_num, count=0;
    srand(time(NULL));
    
    while(1)
    {
        rand_num = rand()%3;
        count++;
        if(elevator_array[rand_num].is_broken == false)
        {
            elevator_array[rand_num].is_broken = true;
            sprintf(notice, "엘리베이터 [%d]호기가 고장났습니다.\n", rand_num+1);
            break;
        }
        
        if(count == 3)
        {
            sprintf(notice, "모든 엘리베이터가 고장났습니다. \n");
            break;
        }
    }
    
    return;
}

void fix_elevator(void)
{
    int i;
    for(i=0; i<3; i++)
    {
        if(elevator_array[i].is_broken == true)
        {
            elevator_array[i].is_broken = false;
            sprintf(notice, "엘리베이터 [%d]호기 수리 완료\n", i+1);
            return;
        }
    }
    
    sprintf(notice, "수리할 엘리베이터가 없습니다.\n");
    return;
}

void set_floor(void)
{
    // 특정 엘리베이터가 그냥 통과할 층 지정
    int number;
    int selected;
    
    while(1)
    {
        printf("몇 호기 엘리베이터의 층을 지정하시겠습니까? (종료: -1)\n");
        scanf("%d", &number);
        
        if(number == -1)
            return;
        
        printf("그냥 통과할 층을 입력하세요. 이미 통과하는 층일 경우 설정이 해제됩니다.(종료: -1)\n");
        scanf("%d", &selected);
        
        if(selected == -1)
            return;
        
        if(pass_floor_array[number-1][selected-1] == 0)
        {
            printf("이제 [%d]호기는 [%d]층을 통과하지 않습니다. \n", number, selected);
            pass_floor_array[number-1][selected-1] = 1;
        }
        else
        {
            printf("이제 [%d]호기는 [%d]층을 통과합니다. \n", number, selected);
            pass_floor_array[number-1][selected-1] = 0;
        }
        
    }
    
    return;
}

void moving_manager(void)
{
    int selected;
    
    printf("=================================\n");
    printf("[1] 고장내기     [2] 고치기\n");
    printf("[3] 운행 층 관리하기 \n");
    printf("=================================\n");
    
    printf("메뉴를 선택하십시오(종료 : -1):  ");
    scanf("%d", &selected);
    
    system("clear");
    switch(selected){
            
        case 1 : break_elevator(); break;
        case 2 : fix_elevator() ; break;
        case 3 : set_floor(); break;
        case -1 : return;
            
        default : printf("잘못된 입력입니다. \n"); break;
    }
    return;

}
void print_elevators(void);

void print_menu(void)
{
    int selected;
    
    while(1)
    {
        print_elevators();
    
        printf("=================================\n");
        printf("[1] 버튼 누르기     [2] 시뮬레이션 관리\n");
        printf("[3] 엘리베이터 운행 관리 \n");
        printf("=================================\n");

        printf("메뉴를 선택하십시오(종료 : -1):  ");
        scanf("%d", &selected);
        
        system("clear");
        switch(selected){
            
            case 1 : button_manager(); break;
            case 2 : simulation_manager() ; break;
            case 3 : moving_manager(); break;
            case -1 : return;

            default : printf("잘못된 입력입니다. \n"); break;
        }
        
    }
    
    return;

}

void print_elevators(void)
{
    int i,j;
    char input='0';
    
    while(1)
    {
        if(strcmp(notice, " ") != 0)
            printf("%s \n", notice);
        
        if(input != '0')
            break;
        for(i=0; i<10; i++)
        {
            printf("         ");
            for(j=0; j<3; j++)
            {
                printf("ㅡㅡㅡㅡ");
            }
            
            printf("\n");
            
            printf("%3dF ", 10-i); // ▼ ▽ ▲	△
            if(button_status[9-i][0] == 1)
                printf("▼ ");
            else if(button_status[9-i][0] == -1)
                printf("  ");
            else
                printf("▽ ");
            
            if(button_status[9-i][1] == 1)
                printf("▲ ");
            else if(button_status[9-i][1] == -1)
                printf("  ");
            else
                printf("△ ");
            
            for(j=0; j<3; j++)
            {
                printf("| ");
                if(elevator_array[j].current_floor == 10-i)
                {
                    if(elevator_array[j].start != NULL)
                    {
                        if(elevator_array[j].current_floor > elevator_array[j].start->element.floor)
                            printf("%s %2d ", "↓", elevator_array[j].start->element.floor); // ↑ ↓
                        else if(elevator_array[j].current_floor < elevator_array[j].start->element.floor)
                            printf("%s %2d ", "↑", elevator_array[j].start->element.floor);
                        else
                            printf("%s %2d ", "-", elevator_array[j].start->element.floor);
                    }
                    else
                        printf(" %s ", " - ");
                }
                else
                    printf("%s", "     ");
            }
            printf("| \n");
        }
        
        printf("         ");
        
        for(j=0; j<3; j++)
        {
            printf("ㅡㅡㅡㅡ");
        }
        printf("\n=================================\n");
        printf("메뉴를 보시려면 아무 키나 누르십시오: \n");
        printf("=================================\n");
        
        struct pollfd mypoll = { STDIN_FILENO, POLLIN|POLLPRI };
        
        if( poll(&mypoll, 1, 1000) )    {
            scanf("%c", &input);
            break;
        }
    
        system("clear");
    }
    
    return;
}



int main(void)
{
    pthread_t pth1, pth2, pth3;
    pthread_t test;
    
    strcpy(notice, " "); // 엘리베이터 애니메이션과 같이 나타날 공지사항 초기화
    
    int a,b,c;
    a = 1;
    b = 2;
    c = 3;
    
    pthread_create(&pth1, NULL, elevator_move, &a);
    pthread_create(&pth2, NULL, elevator_move, &b);
    pthread_create(&pth3, NULL, elevator_move, &c);
    
    print_menu();
    return 0;
}





