// Diani Mamoudou S
// Decembre 2024
// code de compilation : g++ -std=c++20 -pthread -o main main.cpp `pkg-config --cflags --libs opencv4`

#include <thread>
#include <iostream>
#include <cstdlib>
#include <opencv2/opencv.hpp>
#include <syslog.h>
#include <semaphore>
#include <chrono>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>

#define FPS 30
#define SAVE_FILE "annexe/image_capturee.png"

cv::VideoCapture camera;
std::binary_semaphore classification_semaphore(0);
std::binary_semaphore photo_semaphore(0);

void set_nonblocking_mode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

bool is_key_pressed() {
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

void run_python_script() {
    while (true) {
        classification_semaphore.acquire();
        int result = system("/home/diani/rt_projet/bin/python3.10 python.py");
        if (result != 0) {
            syslog(LOG_ERR, "Erreur lors de l'exécution du script Python");
        } else {
            syslog(LOG_INFO, "Classification effectuée avec succès");
        }
    }
}

int initialisation() {
    syslog(LOG_INFO, "****************** Initialisation de la camera ******************");
    camera.open(0);
    if (!camera.isOpened()) {
        syslog(LOG_ERR, "Erreur : Impossible d'ouvrir la camera");
        return -1;
    } else {
        // Configurer la caméra pour fonctionner à 30 FPS
        camera.set(cv::CAP_PROP_FPS, FPS);
        double fps = camera.get(cv::CAP_PROP_FPS);
        if (fps != (double)FPS) {
            return -1;
        } else {
            syslog(LOG_INFO, "FPS configures est %.2f", fps);
            syslog(LOG_INFO, "Initialisation complete");
        }
    }
    return 0;
}



int capture() {
    if (!camera.isOpened()) {
        syslog(LOG_ERR, "Erreur : la camera n'est pas ouverte");
        exit(-1);
    } else {
        cv::Mat frame;
        cv::Mat grayscaleFrame;
        bool success = camera.read(frame);
        if (!success) {
            syslog(LOG_ERR, "Erreur : impossible de capturer une image");
            exit(-1);
        } else {
            cv::cvtColor(frame, grayscaleFrame, cv::COLOR_BGR2GRAY);
            std::string filename = SAVE_FILE;
            success = cv::imwrite(filename, grayscaleFrame);
            if (!success) {
                syslog(LOG_ERR, "Erreur : impossible d'enregistrer l'image");
                exit(-1);
            } else {
                syslog(LOG_INFO, "Image enregistree avec succes");
                return 0;
            }
        }
    }
}

void camera_thread_function() {
    if (initialisation() != 0) {
        syslog(LOG_ERR, "Erreur lors de l'initialisation de la camera");
        exit(-1);
    }
    while (true) {
        photo_semaphore.acquire();
        if (capture() == 0) {
            syslog(LOG_INFO, "Photo prise");
        } else {
            syslog(LOG_ERR, "Erreur lors de la capture d'image");
            exit(-1);
        }
    }
}

void sequencer_script(){
    int i = 0;
    while (true){
        auto start = std::chrono::high_resolution_clock::now();
        photo_semaphore.release();
        usleep(60000);
        classification_semaphore.release();
        std::this_thread::sleep_for(std::chrono::milliseconds(700));
        usleep(240000);
        auto end = std::chrono::high_resolution_clock::now();
        std::cout  << "*********** T" << i << ": " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start) << " Milliseconds ***********" << std::endl;
        i++;
    }
    photo_semaphore.release();
    usleep(60000);
    classification_semaphore.release();
    usleep(100000);
    exit(0);
}


void set_realtime_priority(int pr, int cpu) {
    struct sched_param param;
    param.sched_priority = pr;

    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) != 0) {
        syslog(LOG_ERR, "Erreur : Impossible de definir les parametres de priorite");
        exit(-1);
    } else {
        syslog(LOG_INFO, "Priorite en temps reel definie avec succes");
    }

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);

    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
        syslog(LOG_ERR, "Erreur lors de la définition de l'affinité CPU");
        exit(-1);
    } else {
        syslog(LOG_INFO, "Affinité CPU définie avec succès sur CPU");
    }
}


int main() {
    openlog("Reconnaissance de geste", LOG_PID | LOG_CONS, LOG_USER);

    syslog(LOG_INFO, "Début de programme");

    set_nonblocking_mode();

    std::thread camera_thread ([]() {
        set_realtime_priority(99, 2);
        camera_thread_function(); 
    });

    std::thread python_thread([](){
        set_realtime_priority(99, 3);
        run_python_script();
    });

    usleep(1000000);

    std::thread sequencer_thread([](){
        set_realtime_priority(99, 1);
        sequencer_script();
    });

    while (true) {
        if (is_key_pressed()) {
            char c = getchar();
            if (c == 6) { 
                syslog(LOG_INFO, "Fin du programme détectée");
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    camera.release();

    camera_thread.detach();
    python_thread.detach();
    sequencer_thread.detach();

    syslog(LOG_INFO, "Fin de programme");

    closelog();
    return 0;
}
