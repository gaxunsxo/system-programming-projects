# SIC/XE 어셈블러 및 시뮬레이터 프로젝트


## 📁 프로젝트 개요
이 프로젝트는 Control Section 방식의 SIC/XE 소스를 Object Program Code로 변환하는 어셈블러를 개발하고, 해당 코드를 시뮬레이션하는 GUI 프로그램을 만드는 것을 목표로 합니다. 


## 🛠️ 프로젝트 구성

### Project 1a
- **과제 내용**: Control Section 방식의 SIC/XE 소스를 Object Program Code로 변환하는 어셈블러 개발.
- **목적**: SIC/XE 어셈블러의 동작 이해 및 확장성을 위한 기본 지식 습득.

### Project 1b
- **과제 내용**: 자바 프로젝트 파일을 사용하여 SIC/XE 소스를 Object Program Code로 변환하는 어셈블러 개발.
- **목적**: C와 자바를 비교하며 SIC/XE 어셈블러의 동작 이해.

### Project 1c
- **과제 내용**: 이전 프로젝트 1a/1b 의 개선.
- **목적**: 기존 소스 코드의 부족한 점 보완 및 심도 있는 이해.

### Project 2
- **과제 내용**: SIC/XE 시뮬레이터 개발, Java GUI 프로그램으로 시뮬레이션 과정 시각화.
- **목적**: 생성된 Object Program Code의 동작 과정을 시뮬레이션할 수 있는 GUI 프로그램 개발.

## 📊 필수 GUI 기능 목록
- 프로그램 종료
- 파일 오픈 (파일 오픈 다이얼로그 창)
- 레지스터 영역 표시 (SIC 및 SIC/XE 레지스터 포함)
- 메모리 영역 표시
    - 가상 메모리 직접 표시
    - 메모리 코드 파싱 후 명령어 목록 생성
- 프로그램 정보 표시
- 1 step 및 all step 기능

## 🌐 GUI 예시
- 초기 화면

![image](https://github.com/user-attachments/assets/e2f1f5db-bf37-4fca-820f-794c6612dbb4)

- 파일 오픈 다이얼로그 창

![image](https://github.com/user-attachments/assets/eea4d185-6b3b-4b41-83a1-e296651b4b6f)


## 🛠️ 프로그램 구조도

![image](https://github.com/user-attachments/assets/af6f1eb7-12c3-457d-8165-430dd8c0e704)

### 모듈 구성
- **Visual Simulator**: GUI 방식으로 시뮬레이터의 동작을 보여주는 모듈.
- **SIC/XE Simulator**: 실제 SIC/XE 시뮬레이터 동작을 수행하는 모듈.
- **Resource Manager**: SIC/XE 가상 머신의 메모리와 레지스터 상태 관리 모듈.
- **ObjectCode Loader**: Object code를 메모리에 로드하는 모듈.
- **Instruction Executor**: 명령어 수행을 정의한 모듈.


## ⚠️ 프로젝트 진행 시 유의 사항
- GUI 라이브러리로 Swing을 사용하기 권장. 다른 라이브러리 사용 시, 프로젝트 폴더에 첨부하고 명칭 및 버전을 레포트에 표기할 것.

## 📧 문의
프로젝트에 대한 질문이나 제안이 있으시면 seoge111493@gmail.com 로 문의해 주세요.

