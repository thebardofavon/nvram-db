#ifndef UNIVERSITY_SCHEMA_H
#define UNIVERSITY_SCHEMA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./include/ram_bptree.h"

// Schema definitions
typedef struct {
    int course_id;      // Key
    char name[64];
    int credits;
    char department[32];
} Course;

typedef struct {
    int faculty_id;     // Key
    char name[64];
    char department[32];
    char position[32];
} Faculty;

typedef struct {
    int student_id;     // Key
    char name[64];
    char major[32];
    int year;
} Student;

typedef struct {
    int id;             // Key (auto-generated)
    int faculty_id;
    int course_id;
    char semester[16];
} Teaches;

typedef struct {
    int id;             // Key (auto-generated)
    int student_id;
    int course_id;
    char grade[4];
    char semester[16];
} Takes;

// Serialization functions
char* serialize_course(Course* course, size_t* size) {
    // Format: name|credits|department
    char* buffer = (char *) malloc(128);
    if (!buffer) return NULL;
    
    snprintf(buffer, 128, "%s|%d|%s", 
        course->name, course->credits, course->department);
    
    *size = strlen(buffer) + 1;
    return buffer;
}

Course* deserialize_course(int key, void* data) {
    Course* course = (Course *)malloc(sizeof(Course));
    if (!course) return NULL;
    
    course->course_id = key;
    
    char* str = (char*)data;
    char* token;
    char* rest = str;
    
    // Parse name
    token = strtok_r(rest, "|", &rest);
    if (token) strncpy(course->name, token, sizeof(course->name) - 1);
    
    // Parse credits
    token = strtok_r(rest, "|", &rest);
    if (token) course->credits = atoi(token);
    
    // Parse department
    token = strtok_r(rest, "|", &rest);
    if (token) strncpy(course->department, token, sizeof(course->department) - 1);
    
    return course;
}

char* serialize_faculty(Faculty* faculty, size_t* size) {
    // Format: name|department|position
    char* buffer = (char*)malloc(128);
    if (!buffer) return NULL;
    
    snprintf(buffer, 128, "%s|%s|%s", 
        faculty->name, faculty->department, faculty->position);
    
    *size = strlen(buffer) + 1;
    return buffer;
}

Faculty* deserialize_faculty(int key, void* data) {
    Faculty* faculty = (Faculty *)malloc(sizeof(Faculty));
    if (!faculty) return NULL;
    
    faculty->faculty_id = key;
    
    char* str = (char*)data;
    char* token;
    char* rest = str;
    
    // Parse name
    token = strtok_r(rest, "|", &rest);
    if (token) strncpy(faculty->name, token, sizeof(faculty->name) - 1);
    
    // Parse department
    token = strtok_r(rest, "|", &rest);
    if (token) strncpy(faculty->department, token, sizeof(faculty->department) - 1);
    
    // Parse position
    token = strtok_r(rest, "|", &rest);
    if (token) strncpy(faculty->position, token, sizeof(faculty->position) - 1);
    
    return faculty;
}

char* serialize_student(Student* student, size_t* size) {
    // Format: name|major|year
    char* buffer = (char*)malloc(128);
    if (!buffer) return NULL;
    
    snprintf(buffer, 128, "%s|%s|%d", 
        student->name, student->major, student->year);
    
    *size = strlen(buffer) + 1;
    return buffer;
}

Student* deserialize_student(int key, void* data) {
    Student* student = (Student*)malloc(sizeof(Student));
    if (!student) return NULL;
    
    student->student_id = key;
    
    char* str = (char*)data;
    char* token;
    char* rest = str;
    
    // Parse name
    token = strtok_r(rest, "|", &rest);
    if (token) strncpy(student->name, token, sizeof(student->name) - 1);
    
    // Parse major
    token = strtok_r(rest, "|", &rest);
    if (token) strncpy(student->major, token, sizeof(student->major) - 1);
    
    // Parse year
    token = strtok_r(rest, "|", &rest);
    if (token) student->year = atoi(token);
    
    return student;
}

char* serialize_teaches(Teaches* teaches, size_t* size) {
    // Format: faculty_id|course_id|semester
    char* buffer = (char*)malloc(64);
    if (!buffer) return NULL;
    
    snprintf(buffer, 64, "%d|%d|%s", 
        teaches->faculty_id, teaches->course_id, teaches->semester);
    
    *size = strlen(buffer) + 1;
    return buffer;
}

Teaches* deserialize_teaches(int key, void* data) {
    Teaches* teaches = (Teaches*)malloc(sizeof(Teaches));
    if (!teaches) return NULL;
    
    teaches->id = key;
    
    char* str = (char*)data;
    char* token;
    char* rest = str;
    
    // Parse faculty_id
    token = strtok_r(rest, "|", &rest);
    if (token) teaches->faculty_id = atoi(token);
    
    // Parse course_id
    token = strtok_r(rest, "|", &rest);
    if (token) teaches->course_id = atoi(token);
    
    // Parse semester
    token = strtok_r(rest, "|", &rest);
    if (token) strncpy(teaches->semester, token, sizeof(teaches->semester) - 1);
    
    return teaches;
}

char* serialize_takes(Takes* takes, size_t* size) {
    // Format: student_id|course_id|grade|semester
    char* buffer = (char*)malloc(64);
    if (!buffer) return NULL;
    
    snprintf(buffer, 64, "%d|%d|%s|%s", 
        takes->student_id, takes->course_id, takes->grade, takes->semester);
    
    *size = strlen(buffer) + 1;
    return buffer;
}

Takes* deserialize_takes(int key, void* data) {
    Takes* takes = (Takes*)malloc(sizeof(Takes));
    if (!takes) return NULL;
    
    takes->id = key;
    
    char* str = (char*)data;
    char* token;
    char* rest = str;
    
    // Parse student_id
    token = strtok_r(rest, "|", &rest);
    if (token) takes->student_id = atoi(token);
    
    // Parse course_id
    token = strtok_r(rest, "|", &rest);
    if (token) takes->course_id = atoi(token);
    
    // Parse grade
    token = strtok_r(rest, "|", &rest);
    if (token) strncpy(takes->grade, token, sizeof(takes->grade) - 1);
    
    // Parse semester
    token = strtok_r(rest, "|", &rest);
    if (token) strncpy(takes->semester, token, sizeof(takes->semester) - 1);
    
    return takes;
}

#endif // UNIVERSITY_SCHEMA_H