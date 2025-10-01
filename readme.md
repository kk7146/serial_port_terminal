# 프로젝트 개요
이 프로그램은 PC의 시리얼 포트를 제어하여 외부 장치와 데이터를 송수신할 수 있는 GUI 기반 툴입니다.
C++과 wxWidgets 라이브러리를 기반으로 제작되었습니다.

# 주요 기능
사용 가능한 COM 포트 나열 기능. 포트 열기/닫기 기능.
보레이트(baud rate), 패리티, 데이터 비트, 스톱 비트 등 통신 설정 지원.
사용자가 입력한 문자열 및 명령어 전송.
매크로 설정.

# 어플리케이션 사진
![alt text](image.png)

# 참고한 환경 설정
https://grr1.tistory.com/23
https://blog.naver.com/jdkim2004/223211503446
일부 lib파일은 변경 필요.

# 시리얼 포트 자동 감지
https://blog.naver.com/jangwn119/130040867497
https://yogyui.tistory.com/entry/MFCSetupAPI-%EC%9E%A5%EC%B9%98%EA%B4%80%EB%A6%AC%EC%9E%90Device-Manager-%EC%A0%95%EB%B3%B4-%EC%96%BB%EA%B8%B0

# 추가로 할 일
OnRefresh 함수화
Cntl + 키 이벤트 추가 (현재 B C D)
터미널 제어 문자 기능
frame 쪽에서 getHandle() 호출 손보기
