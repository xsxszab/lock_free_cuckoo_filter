#ifndef MARCOS_H
#define MARCOS_H

/**
 * @file marcos.h
 * @brief this file contains some useful marcos that can make life easier
 */

/**
 * @brief disable copy constructor for a class
 */
#define DISABLE_COPY_CONSTRUCT(class_name)  \
    class_name(const class_name&) = delete; \
    class_name& operator=(const class_name&) = delete;

/**
 * @brief disable move constructor for a class
 */
#define DISABLE_MOVE_CONSTRUCT(class_name) \
    class_name(class_name&&) = delete;     \
    class_name& operator=(class_name&&) = delete;

/**
 * @brief disable both copy and move constructor for a class
 */
#define DISABLE_COPY_AND_MOVE_CONSTRUCT(class_name) \
    DISABLE_COPY_CONSTRUCT(class_name)              \
    DISABLE_MOVE_CONSTRUCT(class_name)

#endif
