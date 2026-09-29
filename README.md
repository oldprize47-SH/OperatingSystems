# 운영체제 실습

[한국어](#korean) · [English](#english)

<a id="korean"></a>
## 한국어

이 저장소에는 Sangheon Park가 C로 작성한 운영체제 수업 과제와 수업에서 제공한 예제가 담겨 있습니다.

[HW1](HW1/hw1_21800275.c)의 프로그램은 다른 명령을 실행하고, 파이프로 그 출력을 받아 출력 내용에서 검색합니다. [과제 README](HW1/README.txt)는 명령 인자와 정확 일치 검색 및 유연한 검색 모드를 설명합니다.

[HW2](HW2/rwlock.c)는 읽기·쓰기 동기화 실습입니다. 입력 순서는 [sequence.txt](HW2/sequence.txt)에 저장되어 있습니다.

[HW3](HW3/mtws.c)는 크기가 제한된 생산자·소비자 큐와 작업자 스레드를 사용해 디렉터리의 파일을 검색합니다. `-b`는 버퍼 크기, `-t`는 스레드 수, `-d`는 디렉터리, `-w`는 검색어를 지정합니다. HW2와 HW3는 2026년 9월 28일 로컬 수업 과제 사본의 내용으로 갱신했습니다.

### 프로젝트 목표

프로세스가 통신하는 방식과 스레드가 공유 데이터 및 파일 검색 작업을 조율하는 방식을 배웁니다.

![프로젝트 목표: operating-systems-labs](docs/goals/project-focus-v1.png)

AI로 생성한 콘셉트 일러스트입니다. 기기의 외형, 인터페이스 배치, 예시 그래픽은 설명을 위한 표현이며, 실제 프로젝트 사진이나 측정 결과가 아닙니다.

### 활용할 수 있는 곳

생산자·소비자와 작업자 패턴은 병렬 파일 검색 도구나 독립적인 작업을 여러 스레드에 나누는 프로그램을 만드는 데 유용한 출발점입니다. 파이프 실습은 별개의 프로세스가 서로 데이터를 전달하는 방식을 보여 줍니다. 이러한 패턴을 장시간 실행되는 도구에 적용하려면 개별 실습 사례를 넘어 오류 처리, 취소, 자원 정리를 확인해야 합니다.

### 한눈에 보기

![운영체제 수업 과제](docs/flowcharts/os.png)

각 행은 서로 독립적인 실습이나 작업 흐름을 설명합니다. 이 저장소 전체가 하나로 연결된 애플리케이션은 아닙니다. [SVG](docs/flowcharts/os.svg)

### 세 실습의 관계

HW1은 프로세스 간 통신을 다룹니다. 자식 프로세스는 요청된 명령을 실행하고 표준 출력을 파이프로 보냅니다. 부모 프로세스는 이 스트림을 읽어 요청된 텍스트를 찾습니다. 정확 일치 검색과 선택적으로 사용하는 문자 기반 모드는 다릅니다. 후자는 비슷한 단어를 찾는 근사 검색이 아니라, 주어진 검색어에 포함된 문자 중 하나라도 있는지 찾습니다.

HW2는 공유 상태에 대한 접근을 다룹니다. 읽기 작업과 쓰기 작업의 순서를 읽어 스레드를 만들고, 세마포어와 읽기 작업자 수를 사용해 이들을 조율합니다. 여러 읽기 작업자는 동시에 접근할 수 있지만, 쓰기 작업자는 배타적 접근이 필요합니다. 출력되는 생성·시작·종료 시각은 실행을 살펴보는 데 도움이 되지만, 그 자체만으로 공정성이나 기아 상태가 없음을 증명하지는 않습니다.

HW3는 파일 검색 작업을 분배합니다. 하나의 생산자가 재귀적으로 파일을 찾아 크기가 제한된 원형 버퍼에 경로를 넣습니다. 소비자 스레드는 경로를 꺼내 각 파일에서 검색어의 출현 횟수를 셉니다. 세마포어는 빈 공간, 큐에 들어간 항목, 버퍼 접근, 공유 합계를 조율합니다. 생산자는 종료 표식을 제공하고, 메인 스레드는 작업자 스레드의 종료를 기다린 뒤 마칩니다.

원래의 학습 맥락을 보존하기 위해 수업 예제도 과제와 함께 남겨 두었습니다. 이 예제들은 제공받은 참고자료이며, 학생이 추가로 작성한 애플리케이션이 아닙니다.

### 빌드

코드는 GCC, pthreads, 세마포어를 사용할 수 있는 Linux/POSIX 환경을 전제로 합니다. 다음은 권장 빌드 명령이며, Windows 포트폴리오 환경에서는 실행하지 않았습니다.

```sh
gcc -Wall -Wextra HW1/hw1_21800275.c -o wspipe
gcc -Wall -Wextra -pthread HW2/rwlock.c -o rwlock
gcc -Wall -Wextra -pthread HW3/mtws.c -o mtws
```

의도된 Linux 환경에서 컴파일한 뒤, 저장소 루트에서 실행할 수 있는 간단한 예시는 다음과 같습니다.

```sh
./wspipe 'ls -al' README
./rwlock HW2/sequence.txt
./mtws -b 8 -t 2 -d HW1 -w main
```

첫 번째 프로그램은 전달받은 명령을 실행합니다. 세 번째 프로그램은 지정된 디렉터리를 검색하므로, 출력을 처음 살펴볼 때는 작은 테스트 폴더를 선택하세요. 이 명령들은 소스에서 도출한 사용 예시이며, Linux에서 새로 실행해 검증한 결과가 아닙니다. HW2는 Windows에서 MinGW GCC 14.2로 컴파일했습니다. Linux 환경을 사용할 수 없었으므로, 전체 POSIX 프로그램과 동시성 동작은 검증하지 못했습니다. 아카이브에 있는 생성된 바이너리는 과거 파일이며, 여러 환경에서 사용할 수 있는 이식 가능한 빌드가 아닙니다. 새로운 동시성 스트레스 테스트나 공정성 측정은 수행하지 않았습니다.

`Source_Codes_for_Ch*` 디렉터리에는 제가 직접 구현한 코드가 아니라 수업용 예제가 들어 있습니다.

[원본 저장소](https://github.com/oldprize47/2025_OS). 원래 이력과 저작자 표기를 유지합니다.

---

<a id="english"></a>
## English

**Operating Systems Labs**

This repository contains operating-systems coursework by Sangheon Park in C, along with examples supplied for the course.

In [HW1](HW1/hw1_21800275.c), a program starts another command, receives its output through a pipe and searches that output. The [assignment README](HW1/README.txt) describes the command arguments and the exact and flexible search modes.

[HW2](HW2/rwlock.c) is a reader/writer synchronisation exercise. Its input sequence is stored in [sequence.txt](HW2/sequence.txt).

[HW3](HW3/mtws.c) searches files in a directory with a bounded producer/consumer queue and worker threads. It accepts `-b` for buffer size, `-t` for thread count, `-d` for directory and `-w` for the search word. HW2 and HW3 were updated from the local coursework copy on 28 September 2026.

### Project goal

Learn how processes communicate and how threads coordinate shared data and file-search work.

![Project goal: operating-systems-labs](docs/goals/project-focus-v1.png)

AI-generated concept illustration. Device appearance, interface layout and example graphics are illustrative, not project photographs or measured results.

### Where it could be used

The producer-consumer and worker patterns are useful starting points for parallel file-search tools and other programs that divide independent jobs among threads. The pipe exercises illustrate how separate processes can pass data to one another. Applying these patterns to a longer-running tool would require checking error handling, cancellation and resource cleanup beyond the individual lab cases.

### At a glance

![Operating-systems coursework](docs/flowcharts/os.png)

Each row describes an independent exercise or workflow; the repository is not one connected application. [SVG](docs/flowcharts/os.svg)

### How the three exercises fit together

HW1 is about communication between processes. The child runs the requested command and redirects its standard output into a pipe. The parent reads that stream and finds the requested text. Exact matching and the optional character-based mode are different: the latter looks for any character from the supplied search word, rather than doing approximate word matching.

HW2 is about access to shared state. It reads a sequence of reader and writer jobs, creates threads and coordinates them with semaphores and a reader count. Multiple readers can share access, while a writer needs exclusive access. The printed creation, start and end times help inspect an execution; they do not by themselves prove fairness or freedom from starvation.

HW3 distributes file-search work. One producer recursively discovers files and puts paths into a bounded circular buffer. Consumer threads remove paths and count the search word in each file. Semaphores coordinate empty spaces, queued items, buffer access and shared totals. The producer supplies termination markers and the main thread joins the workers before finishing.

The course examples remain beside the assignments to preserve the original learning context. They are supplied reference material rather than additional student-authored applications.

### Building

The code expects a Linux/POSIX environment with GCC, pthreads and semaphores. The following are suggested build commands; they have not been run in the Windows portfolio environment:

```sh
gcc -Wall -Wextra HW1/hw1_21800275.c -o wspipe
gcc -Wall -Wextra -pthread HW2/rwlock.c -o rwlock
gcc -Wall -Wextra -pthread HW3/mtws.c -o mtws
```

From the repository root in the intended Linux environment, these are small example invocations after compilation:

```sh
./wspipe 'ls -al' README
./rwlock HW2/sequence.txt
./mtws -b 8 -t 2 -d HW1 -w main
```

The first program executes the command passed to it. The third searches the specified directory, so select a small test folder when first examining its output. These are usage examples derived from the source, not freshly verified Linux runs. HW2 compiled with MinGW GCC 14.2 on Windows. A Linux environment was not available, so the full POSIX programs and concurrency behavior remain unverified. The generated binaries in the archive are historical files, not portable builds. No new concurrency stress test or fairness measurement has been performed.

The `Source_Codes_for_Ch*` directories contain teaching examples rather than my original implementations.

[Original repository](https://github.com/oldprize47/2025_OS). Original history and attribution are retained.
