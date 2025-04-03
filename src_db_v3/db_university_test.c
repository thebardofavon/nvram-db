#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/ram_bptree.h"
#include "include/free_space.h"
#include "include/wal.h"
#include "university_schema.h"

// Function declarations for helper functions
static int insert_course(Table *course_table, int txn_id, int course_id, const char *name, int credits, const char *department);
static int insert_faculty(Table *faculty_table, int txn_id, int faculty_id, const char *name, const char *department, const char *position);
static int insert_student(Table *student_table, int txn_id, int student_id, const char *name, const char *major, int year);
static int insert_teaches(Table *teaches_table, int txn_id, int *next_teaches_id, int faculty_id, int course_id, const char *semester);
static int insert_takes(Table *takes_table, int txn_id, int *next_takes_id, int student_id, int course_id, const char *grade, const char *semester);

// Function to test the university database
void test_university_db() {
    // Initialize the database
    db_init();
    
    // Table pointers
    Table *course_table = NULL;
    Table *faculty_table = NULL;
    Table *student_table = NULL;
    Table *teaches_table = NULL;
    Table *takes_table = NULL;
    
    // Counter for relationship table IDs
    int next_teaches_id = 1;
    int next_takes_id = 1;
    
    printf("Creating tables...\n");
    
    // Create course table
    int course_id = db_create_table("course");
    if (course_id >= 0) {
        printf("Created course table with ID: %d\n", course_id);
        course_table = db_open_table("course");
    } else {
        printf("Failed to create course table\n");
        db_shutdown();
        return;
    }
    
    // Create faculty table
    int faculty_id = db_create_table("faculty");
    if (faculty_id >= 0) {
        printf("Created faculty table with ID: %d\n", faculty_id);
        faculty_table = db_open_table("faculty");
    } else {
        printf("Failed to create faculty table\n");
        db_shutdown();
        return;
    }
    
    // Create student table
    int student_id = db_create_table("student");
    if (student_id >= 0) {
        printf("Created student table with ID: %d\n", student_id);
        student_table = db_open_table("student");
    } else {
        printf("Failed to create student table\n");
        db_shutdown();
        return;
    }
    
    // Create teaches table
    int teaches_id = db_create_table("teaches");
    if (teaches_id >= 0) {
        printf("Created teaches table with ID: %d\n", teaches_id);
        teaches_table = db_open_table("teaches");
    } else {
        printf("Failed to create teaches table\n");
        db_shutdown();
        return;
    }
    
    // Create takes table
    int takes_id = db_create_table("takes");
    if (takes_id >= 0) {
        printf("Created takes table with ID: %d\n", takes_id);
        takes_table = db_open_table("takes");
    } else {
        printf("Failed to create takes table\n");
        db_shutdown();
        return;
    }
    
    // Begin a transaction
    int txn_id = db_begin_transaction();
    printf("Started transaction: %d\n", txn_id);
    
    // Insert sample data
    // Insert courses
    insert_course(course_table, txn_id, 101, "Introduction to Computer Science", 3, "Computer Science");
    insert_course(course_table, txn_id, 102, "Data Structures", 4, "Computer Science");
    insert_course(course_table, txn_id, 201, "Database Systems", 3, "Computer Science");
    insert_course(course_table, txn_id, 301, "Advanced Algorithms", 4, "Computer Science");
    insert_course(course_table, txn_id, 401, "Machine Learning", 3, "Computer Science");
    insert_course(course_table, txn_id, 151, "Calculus I", 4, "Mathematics");
    insert_course(course_table, txn_id, 152, "Calculus II", 4, "Mathematics");
    
    // Insert faculty
    insert_faculty(faculty_table, txn_id, 1001, "John Smith", "Computer Science", "Professor");
    insert_faculty(faculty_table, txn_id, 1002, "Jane Doe", "Computer Science", "Associate Professor");
    insert_faculty(faculty_table, txn_id, 1003, "Robert Johnson", "Mathematics", "Professor");
    
    // Insert students
    insert_student(student_table, txn_id, 10001, "Alice Williams", "Computer Science", 2);
    insert_student(student_table, txn_id, 10002, "Bob Brown", "Computer Science", 3);
    insert_student(student_table, txn_id, 10003, "Charlie Davis", "Mathematics", 2);
    insert_student(student_table, txn_id, 10004, "Diana Miller", "Computer Science", 4);
    insert_student(student_table, txn_id, 10005, "Edward Wilson", "Mathematics", 1);
    
    // Insert teaching relationships
    insert_teaches(teaches_table, txn_id, &next_teaches_id, 1001, 101, "Fall 2024");
    insert_teaches(teaches_table, txn_id, &next_teaches_id, 1001, 301, "Fall 2024");
    insert_teaches(teaches_table, txn_id, &next_teaches_id, 1002, 102, "Fall 2024");
    insert_teaches(teaches_table, txn_id, &next_teaches_id, 1002, 201, "Spring 2025");
    insert_teaches(teaches_table, txn_id, &next_teaches_id, 1003, 151, "Fall 2024");
    insert_teaches(teaches_table, txn_id, &next_teaches_id, 1003, 152, "Spring 2025");
    
    // Insert student enrollments
    insert_takes(takes_table, txn_id, &next_takes_id, 10001, 101, "A", "Fall 2024");
    insert_takes(takes_table, txn_id, &next_takes_id, 10001, 151, "B+", "Fall 2024");
    insert_takes(takes_table, txn_id, &next_takes_id, 10002, 102, "A-", "Fall 2024");
    insert_takes(takes_table, txn_id, &next_takes_id, 10002, 201, "B", "Spring 2025");
    insert_takes(takes_table, txn_id, &next_takes_id, 10003, 101, "B", "Fall 2024");
    insert_takes(takes_table, txn_id, &next_takes_id, 10003, 151, "A", "Fall 2024");
    insert_takes(takes_table, txn_id, &next_takes_id, 10004, 301, "A-", "Fall 2024");
    insert_takes(takes_table, txn_id, &next_takes_id, 10005, 151, "B+", "Fall 2024");
    
    // Commit the transaction
    if (db_commit_transaction(txn_id)) {
        printf("Transaction committed successfully\n");
    } else {
        printf("Failed to commit transaction\n");
        db_shutdown();
        return;
    }
    
    // Start a new transaction for queries
    txn_id = db_begin_transaction();
    
    // Print all courses
    printf("\n=== All Courses ===\n");
    int key = -1;
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
    
    // Print all faculty
    printf("\n=== All Faculty ===\n");
    key = -1;
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
    
    // Print all students
    printf("\n=== All Students ===\n");
    key = -1;
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
    
    // Example query: Find courses by department
    printf("\n=== Courses in Department: Computer Science ===\n");
    key = -1;
    int count = 0;
    while ((key = db_get_next_row(course_table, key)) != -1) {
        size_t size;
        void *data = db_get_row(course_table, txn_id, key, &size);
        if (data) {
            Course *course = deserialize_course(key, data);
            if (strcmp(course->department, "Computer Science") == 0) {
                printf("Course ID: %d, Name: %s, Credits: %d\n",
                       course->course_id, course->name, course->credits);
                count++;
            }
            free(course);
        }
    }
    
    // Example query: Find courses taught by faculty
    printf("\n=== Courses Taught by Faculty: John Smith (ID: 1001) ===\n");
    key = -1;
    count = 0;
    while ((key = db_get_next_row(teaches_table, key)) != -1) {
        size_t size;
        void *data = db_get_row(teaches_table, txn_id, key, &size);
        if (data) {
            Teaches *teaches = deserialize_teaches(key, data);
            if (teaches->faculty_id == 1001) {
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
    
    printf("\nUniversity database test completed successfully\n");
}

// Helper function for course insertion
static int insert_course(Table *course_table, int txn_id, int course_id, const char *name, int credits, const char *department) {
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

// Helper function for faculty insertion
static int insert_faculty(Table *faculty_table, int txn_id, int faculty_id, const char *name, const char *department, const char *position) {
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

// Helper function for student insertion
static int insert_student(Table *student_table, int txn_id, int student_id, const char *name, const char *major, int year) {
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

// Helper function for teaches relationship insertion
static int insert_teaches(Table *teaches_table, int txn_id, int *next_teaches_id, int faculty_id, int course_id, const char *semester) {
    Teaches teaches;
    teaches.id = (*next_teaches_id)++;
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

// Helper function for takes relationship insertion
static int insert_takes(Table *takes_table, int txn_id, int *next_takes_id, int student_id, int course_id, const char *grade, const char *semester) {
    Takes takes;
    takes.id = (*next_takes_id)++;
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

// New entry function that doesn't conflict with existing main
int university_main() {
    printf("Starting University Database Test\n");
    test_university_db();
    return 0;
}