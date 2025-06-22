# crossyfrog

## 프로젝트 개요
`Crossy Frog`는 개구리가 도로를 건너며 목적지에 도달하는 게임으로, 
C++ 객체지향 프로그래밍(OOP) 개념을 학습하고 실습하는 것을 목표로 하였습니다.

게임은 `Home`(하단)에서 시작하여 `School`(상단)까지 도달한 뒤 다시 돌아오면 점수를 획득합니다.  
점수가 증가할수록 도로 위 차량의 수가 늘어나며, 난이도가 상승합니다.

본 프로젝트는 기존 C 기반 로직을 **C++의 클래스, 상속, 다형성**을 활용하여 구조화하였습니다.

## 사용 기술
- **C++ 객체지향 프로그래밍**
  
  `Object`라는 추상 클래스를 기반으로, `Car`와 `Frog` 클래스가 이를 상속받아 각각의 `draw()` 함수를 오버라이딩하는 구조로 구성되었습니다.
- **키 입력 처리**

  조이스틱 입력은 `Jog_key`와 `Jog_key_in`이라는 전역 변수로 관리됩니다.
  
  사용자가 조이스틱을 입력하면, 해당 방향에 따라 `Frog` 객체의 `moveUp()`, `moveDown()`, `moveLeft()`, `moveRight()` 멤버 함수가 실행되어 개구리의 좌표가 실시간으로 변경됩니다.
  
  입력은 타이머 및 인터럽트 기반으로 동작하여, 빠르고 정확한 조작이 가능합니다.
- **화면 출력**
  
  화면 출력은 `Lcd_Draw_Box()`, `Lcd_Printf()` 등 LCD 전용 API 함수를 통해 직접 수행됩니다.

## 외관 사진
![image](https://github.com/user-attachments/assets/f06cad15-802a-406c-87a9-9713f940f387)
## 시연 영상
- 일반 시연
https://youtu.be/gVar57MCcvc?si=SoJKUfi_6djEawM6
- 실패 영상
  https://youtu.be/xvSkDguM6lE
- 치트 모드
  https://youtu.be/4xUqFMPrRLs

