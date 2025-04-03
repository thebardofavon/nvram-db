#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./include/ram_bptree.h"
#include "./include/free_space.h"
#include "./include/wal.h"
#include "university_schema.h"

// Global tables
Table *course_table = NULL;
Table *faculty_table = NULL;
Table *student_table = NULL;
Table *teaches_table = NULL;
Table *takes_table = NULL;

// Counter for relationship table IDs
int next_teaches_id = 1;
int next_takes_id = 1;

// Function to create all tables
void create_tables() {
    printf("Creating tables...\n");
    
    // Create course table
    int course_id = db_create_table("course");
    if (course_id >= 0) {
        printf("Created course table with ID: %d\n", course_id);
        course_table = db_open_table("course");
    } else {
        printf("Failed to create course table\n");
    }
    
    // Create faculty table
    int faculty_id = db_create_table("faculty");
    if (faculty_id >= 0) {
        printf("Created faculty table with ID: %d\n", faculty_id);
        faculty_table = db_open_table("faculty");
    } else {
        printf("Failed to create faculty table\n");
    }
    
    // Create student table
    int student_id = db_create_table("student");
    if (student_id >= 0) {
        printf("Created student table with ID: %d\n", student_id);
        student_table = db_open_table("student");
    } else {
        printf("Failed to create student table\n");
    }
    
    // Create teaches table
    int teaches_id = db_create_table("teaches");
    if (teaches_id >= 0) {
        printf("Created teaches table with ID: %d\n", teaches_id);
        teaches_table = db_open_table("teaches");
    } else {
        printf("Failed to create teaches table\n");
    }
    
    // Create takes table
    int takes_id = db_create_table("takes");
    if (takes_id >= 0) {
        printf("Created takes table with ID: %d\n", takes_id);
        takes_table = db_open_table("takes");
    } else {
        printf("Failed to create takes table\n");
    }
}

// Function to insert a course
int insert_course(int txn_id, int course_id, const char *name, int credits, const char *department) {
    if (!course_table) {
        printf("Course table not open\n");
        return 0;
    }
    
    Course course;
    course.course_id = course_id;
    strncpy(course.name, name, sizeof(course.name) - 1);
    course.credits = credits;
    strncpy(course.department, department, sizeof(course.department) - 1);
    
    size_t size;
    char *data = serialize_course(&course, &size);
    if (!data) {
        printf("Failed to serialize course data\n");
        return 0;
    }
    
    int success = db_put_row(course_table, txn_id, course_id, data, size);
    free(data);
    
    if (success) {
        printf("Inserted course: %d, %s\n", course_id, name);
        return 1;
    } else {
        printf("Failed to insert course: %d\n", course_id);
        return 0;
    }
}

// Function to insert a faculty member
int insert_faculty(int txn_id, int faculty_id, const char *name, const char *department, const char *position) {
    if (!faculty_table) {
        printf("Faculty table not open\n");
        return 0;
    }
    
    Faculty faculty;
    faculty.faculty_id = faculty_id;
    strncpy(faculty.name, name, sizeof(faculty.name) - 1);
    strncpy(faculty.department, department, sizeof(faculty.department) - 1);
    strncpy(faculty.position, position, sizeof(faculty.position) - 1);
    
    size_t size;
    char *data = serialize_faculty(&faculty, &size);
    if (!data) {
        printf("Failed to serialize faculty data\n");
        return 0;
    }
    
    int success = db_put_row(faculty_table, txn_id, faculty_id, data, size);
    free(data);
    
    if (success) {
        printf("Inserted faculty: %d, %s\n", faculty_id, name);
        return 1;
    } else {
        printf("Failed to insert faculty: %d\n", faculty_id);
        return 0;
    }
}

// Function to insert a student
int insert_student(int txn_id, int student_id, const char *name, const char *major, int year) {
    if (!student_table) {
        printf("Student table not open\n");
        return 0;
    }
    
    Student student;
    student.student_id = student_id;
    strncpy(student.name, name, sizeof(student.name) - 1);
    strncpy(student.major, major, sizeof(student.major) - 1);
    student.year = year;
    
    size_t size;
    char *data = serialize_student(&student, &size);
    if (!data) {
        printf("Failed to serialize student data\n");
        return 0;
    }
    
    int success = db_put_row(student_table, txn_id, student_id, data, size);
    free(data);
    
    if (success) {
        printf("Inserted student: %d, %s\n", student_id, name);
        return 1;
    } else {
        printf("Failed to insert student: %d\n", student_id);
        return 0;
    }
}

// Function to insert a teaching relationship
int insert_teaches(int txn_id, int faculty_id, int course_id, const char *semester) {
    if (!teaches_table) {
        printf("Teaches table not open\n");
        return 0;
    }
    
    Teaches teaches;
    teaches.id = next_teaches_id++;
    teaches.faculty_id = faculty_id;
    teaches.course_id = course_id;
    strncpy(teaches.semester, semester, sizeof(teaches.semester) - 1);
    
    size_t size;
    char *data = serialize_teaches(&teaches, &size);
    if (!data) {
        printf("Failed to serialize teaches data\n");
        return 0;
    }
    
    int success = db_put_row(teaches_table, txn_id, teaches.id, data, size);
    free(data);
    
    if (success) {
        printf("Inserted teaches: faculty %d teaches course %d in %s\n", 
               faculty_id, course_id, semester);
        return 1;
    } else {
        printf("Failed to insert teaches relationship\n");
        return 0;
    }
}

// Function to insert a student enrollment
int insert_takes(int txn_id, int student_id, int course_id, const char *grade, const char *semester) {
    if (!takes_table) {
        printf("Takes table not open\n");
        return 0;
    }
    
    Takes takes;
    takes.id = next_takes_id++;
    takes.student_id = student_id;
    takes.course_id = course_id;
    strncpy(takes.grade, grade, sizeof(takes.grade) - 1);
    strncpy(takes.semester, semester, sizeof(takes.semester) - 1);
    
    size_t size;
    char *data = serialize_takes(&takes, &size);
    if (!data) {
        printf("Failed to serialize takes data\n");
        return 0;
    }
    
    int success = db_put_row(takes_table, txn_id, takes.id, data, size);
    free(data);
    
    if (success) {
        printf("Inserted takes: student %d takes course %d with grade %s in %s\n", 
               student_id, course_id, grade, semester);
        return 1;
    } else {
        printf("Failed to insert takes relationship\n");
        return 0;
    }
}

// Function to print all courses
void print_all_courses(int txn_id) {
    printf("\n=== All Courses ===\n");
    
    int key = -1;  // Start from the beginning
    while ((key = db_get_next_row(course_table, key)) != -1) {
        size_t size;
        void *data = db_get_row(course_table, txn_id, key, &size);
        if (data) {
            Course *course = deserialize_course(key, data);
            printf("Course ID: %d, Name: %s, Credits: %d, Department: %s\n",
                   course->course_id, course->name, course->credits, course->department);
            free(course);
        }
    }
}

// Function to print all faculty
void print_all_faculty(int txn_id) {
    printf("\n=== All Faculty ===\n");
    
    int key = -1;  // Start from the beginning
    while ((key = db_get_next_row(faculty_table, key)) != -1) {
        size_t size;
        void *data = db_get_row(faculty_table, txn_id, key, &size);
        if (data) {
            Faculty *faculty = deserialize_faculty(key, data);
            printf("Faculty ID: %d, Name: %s, Department: %s, Position: %s\n",
                   faculty->faculty_id, faculty->name, faculty->department, faculty->position);
            free(faculty);
        }
    }
}

// Function to print all students
void print_all_students(int txn_id) {
    printf("\n=== All Students ===\n");
    
    int key = -1;  // Start from the beginning
    while ((key = db_get_next_row(student_table, key)) != -1) {
        size_t size;
        void *data = db_get_row(student_table, txn_id, key, &size);
        if (data) {
            Student *student = deserialize_student(key, data);
            printf("Student ID: %d, Name: %s, Major: %s, Year: %d\n",
                   student->student_id, student->name, student->major, student->year);
            free(student);
        }
    }
}

// Function to print all teaching relationships
void print_all_teaches(int txn_id) {
    printf("\n=== All Teaching Assignments ===\n");
    
    int key = -1;  // Start from the beginning
    while ((key = db_get_next_row(teaches_table, key)) != -1) {
        size_t size;
        void *data = db_get_row(teaches_table, txn_id, key, &size);
        if (data) {
            Teaches *teaches = deserialize_teaches(key, data);
            
            // Get faculty name
            size_t faculty_size;
            void *faculty_data = db_get_row(faculty_table, txn_id, teaches->faculty_id, &faculty_size);
            Faculty *faculty = NULL;
            if (faculty_data) {
                faculty = deserialize_faculty(teaches->faculty_id, faculty_data);
            }
            
            // Get course name
            size_t course_size;
            void *course_data = db_get_row(course_table, txn_id, teaches->course_id, &course_size);
            Course *course = NULL;
            if (course_data) {
                course = deserialize_course(teaches->course_id, course_data);
            }
            
            printf("ID: %d, Faculty: %s (%d), Course: %s (%d), Semester: %s\n",
                   teaches->id,
                   faculty ? faculty->name : "Unknown",
                   teaches->faculty_id,
                   course ? course->name : "Unknown",
                   teaches->course_id,
                   teaches->semester);
            
            if (faculty) free(faculty);
            if (course) free(course);
            free(teaches);
        }
    }
}

// Function to print all student enrollments
void print_all_takes(int txn_id) {
    printf("\n=== All Student Enrollments ===\n");
    
    int key = -1;  // Start from the beginning
    while ((key = db_get_next_row(takes_table, key)) != -1) {
        size_t size;
        void *data = db_get_row(takes_table, txn_id, key, &size);
        if (data) {
            Takes *takes = deserialize_takes(key, data);
            
            // Get student name
            size_t student_size;
            void *student_data = db_get_row(student_table, txn_id, takes->student_id, &student_size);
            Student *student = NULL;
            if (student_data) {
                student = deserialize_student(takes->student_id, student_data);
            }
            
            // Get course name
            size_t course_size;
            void *course_data = db_get_row(course_table, txn_id, takes->course_id, &course_size);
            Course *course = NULL;
            if (course_data) {
                course = deserialize_course(takes->course_id, course_data);
            }
            
            printf("ID: %d, Student: %s (%d), Course: %s (%d), Grade: %s, Semester: %s\n",
                   takes->id,
                   student ? student->name : "Unknown",
                   takes->student_id,
                   course ? course->name : "Unknown",
                   takes->course_id,
                   takes->grade,
                   takes->semester);
            
            if (student) free(student);
            if (course) free(course);
            free(takes);
        }
    }
}

// Function to find courses by department
void find_courses_by_department(int txn_id, const char *department) {
    printf("\n=== Courses in Department: %s ===\n", department);
    
    int count = 0;
    int key = -1;  // Start from the beginning
    while ((key = db_get_next_row(course_table, key)) != -1) {
        size_t size;
        void *data = db_get_row(course_table, txn_id, key, &size);
        if (data) {
            Course *course = deserialize_course(key, data);
            if (strcmp(course->department, department) == 0) {
                printf("Course ID: %d, Name: %s, Credits: %d\n",
                       course->course_id, course->name, course->credits);
                count++;
            }
            free(course);
        }
    }
    
    if (count == 0) {
        printf("No courses found in department %s\n", department);
    }
}

// Function to find courses taught by a faculty member
void find_courses_by_faculty(int txn_id, int faculty_id) {
    // Get faculty name first
    size_t faculty_size;
    void *faculty_data = db_get_row(faculty_table, txn_id, faculty_id, &faculty_size);
    if (!faculty_data) {
        printf("Faculty with ID %d not found\n", faculty_id);
        return;
    }
    
    Faculty *faculty = deserialize_faculty(faculty_id, faculty_data);
    printf("\n=== Courses Taught by Faculty: %s (ID: %d) ===\n", 
           faculty->name, faculty_id);
    free(faculty);
    
    int count = 0;
    int key = -1;  // Start from the beginning
    while ((key = db_get_next_row(teaches_table, key)) != -1) {
        size_t size;
        void *data = db_get_row(teaches_table, txn_id, key, &size);
        if (data) {
            Teaches *teaches = deserialize_teaches(key, data);
            if (teaches->faculty_id == faculty_id) {
                // Get course details
                size_t course_size;
                void *course_data = db_get_row(course_table, txn_id, teaches->course_id, &course_size);
                if (course_data) {
                    Course *course = deserialize_course(teaches->course_id, course_data);
                    printf("Course ID: %d, Name: %s, Semester: %s\n",
                           course->course_id, course->name, teaches->semester);
                    free(course);
                    count++;
                }
            }
            free(teaches);
        }
    }
    
    if (count == 0) {
        printf("No courses found for faculty ID %d\n", faculty_id);
    }
}

// Function to find students enrolled in a course
void find_students_by_course(int txn_id, int course_id) {
    // Get course name first
    size_t course_size;
    void *course_data = db_get_row(course_table, txn_id, course_id, &course_size);
    if (!course_data) {
        printf("Course with ID %d not found\n", course_id);
        return;
    }
    
    Course *course = deserialize_course(course_id, course_data);
    printf("\n=== Students Enrolled in Course: %s (ID: %d) ===\n", 
           course->name, course_id);
    free(course);
    
    int count = 0;
    int key = -1;  // Start from the beginning
    while ((key = db_get_next_row(takes_table, key)) != -1) {
        size_t size;
        void *data = db_get_row(takes_table, txn_id, key, &size);
        if (data) {
            Takes *takes = deserialize_takes(key, data);
            if (takes->course_id == course_id) {
                // Get student details
                size_t student_size;
                void *student_data = db_get_row(student_table, txn_id, takes->student_id, &student_size);
                if (student_data) {
                    Student *student = deserialize_student(takes->student_id, student_data);
                    printf("Student ID: %d, Name: %s, Grade: %s, Semester: %s\n",
                           student->student_id, student->name, takes->grade, takes->semester);
                    free(student);
                    count++;
                }
            }
            free(takes);
        }
    }
    
    if (count == 0) {
        printf("No students found enrolled in course ID %d\n", course_id);
    }
}

// Main function with test data
int main() {
    // Initialize the database
    db_init();
    
    // Create all tables
    create_tables();
    
    // Begin a transaction
    int txn_id = db_begin_transaction();
    printf("Started transaction: %d\n", txn_id);
    
    // Insert courses
    insert_course(txn_id, 101, "Introduction to Computer Science", 3, "Computer Science");
    insert_course(txn_id, 102, "Data Structures", 4, "Computer Science");
    insert_course(txn_id, 201, "Database Systems", 3, "Computer Science");
    insert_course(txn_id, 301, "Advanced Algorithms", 4, "Computer Science");
    insert_course(txn_id, 401, "Machine Learning", 3, "Computer Science");
    insert_course(txn_id, 151, "Calculus I", 4, "Mathematics");
    insert_course(txn_id, 152, "Calculus II", 4, "Mathematics");
    
    // Insert faculty
    insert_faculty(txn_id, 1001, "John Smith", "Computer Science", "Professor");
    insert_faculty(txn_id, 1002, "Jane Doe", "Computer Science", "Associate Professor");
    insert_faculty(txn_id, 1003, "Robert Johnson", "Mathematics", "Professor");
    
    // Insert students
    insert_student(txn_id, 10001, "Alice Williams", "Computer Science", 2);
    insert_student(txn_id, 10002, "Bob Brown", "Computer Science", 3);
    insert_student(txn_id, 10003, "Charlie Davis", "Mathematics", 2);
    insert_student(txn_id, 10004, "Diana Miller", "Computer Science", 4);
    insert_student(txn_id, 10005, "Edward Wilson", "Mathematics", 1);
    
    // Insert teaching relationships
    insert_teaches(txn_id, 1001, 101, "Fall 2024");
    insert_teaches(txn_id, 1001, 301, "Fall 2024");
    insert_teaches(txn_id, 1002, 102, "Fall 2024");
    insert_teaches(txn_id, 1002, 201, "Spring 2025");
    insert_teaches(txn_id, 1003, 151, "Fall 2024");
    insert_teaches(txn_id, 1003, 152, "Spring 2025");
    
    // Insert student enrollments
    insert_takes(txn_id, 10001, 101, "A", "Fall 2024");
    insert_takes(txn_id, 10001, 151, "B+", "Fall 2024");
    insert_takes(txn_id, 10002, 102, "A-", "Fall 2024");
    insert_takes(txn_id, 10002, 201, "B", "Spring 2025");
    insert_takes(txn_id, 10003, 101, "B", "Fall 2024");
    insert_takes(txn_id, 10003, 151, "A", "Fall 2024");
    insert_takes(txn_id, 10004, 301, "A-", "Fall 2024");
    insert_takes(txn_id, 10005, 151, "B+", "Fall 2024");
    
    // Commit the transaction
    if (db_commit_transaction(txn_id)) {
        printf("Transaction committed successfully\n");
    } else {
        printf("Failed to commit transaction\n");
    }
    
    // Start a new transaction for queries
    txn_id = db_begin_transaction();
    
    // Print all data
    print_all_courses(txn_id);
    print_all_faculty(txn_id);
    print_all_students(txn_id);
    print_all_teaches(txn_id);
    print_all_takes(txn_id);
    
    // Example queries
    find_courses_by_department(txn_id, "Computer Science");
    find_courses_by_faculty(txn_id, 1001);
    find_students_by_course(txn_id, 101);
    
    // Commit the query transaction
    db_commit_transaction(txn_id);
    
    // Close all tables
    db_close_table(course_table);
    db_close_table(faculty_table);
    db_close_table(student_table);
    db_close_table(teaches_table);
    db_close_table(takes_table);
    
    // Shutdown the database
    db_shutdown();
    
    printf("\nDatabase test completed successfully\n");
    return 0;
}