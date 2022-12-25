# LOGIC CIRCUIT DESIGN & LAB  

## 회로도
![termProject](https://user-images.githubusercontent.com/99540674/209459052-ea0634d9-90d4-42ef-82df-a7aee68d55fc.jpg)  
  
  
## 구동영상
<img width="100%" src="https://user-images.githubusercontent.com/99540674/209461675-451373a4-417f-4dbc-bbd3-83d75407a730.gif"/>

## 동작설명
1. *버튼을 누르기 전 까지는 JK FlipFlop을 이용해 모든 KEYPAD의 입력을 막는다.  
2. *버튼을 누르면 STEP모터가 시계반대방량으로 회전하며 프로그램의 동작을 알린다.
3. 그 후 4번의 숫자가 입력될 때마다, 기회가 5에서 1씩 떨어진다. 
4. LED는 5-4번까지는 초록색, 3-1까지는 파란색, 기회를 전부 소진하면 빨간색이 점등된다.
5. 설정되어 있는 비밀번호의 일치하는 자리 수가 왼쪽 7-segment에 출력된다.
6. 4자리의 비밀번호를 전부 맞췄을 경우 8개의 미니LED가 점등되고, LED가 백색으로 점등된다.
7. #버튼을 누를 시 모든 입력값과 출력값이 RESET된다. (비밀번호의 입력을 막는 *버튼도 RESET)
