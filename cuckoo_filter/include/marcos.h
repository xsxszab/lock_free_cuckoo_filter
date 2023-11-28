#ifndef MARCOS_H
#define MARCOS_H

#define DISABLE_COPY_CONSTRUCT(class_name)  \
    class_name(const class_name&) = delete; \
    class_name& operator=(const class_name&) = delete;

#define DISABLE_MOVE_CONSTRUCT(class_name) \
    class_name(class_name&&) = delete;     \
    class_name& operator=(class_name&&) = delete;

#define DISABLE_COPY_AND_MOVE_CONSTRUCT(class_name) \
    DISABLE_COPY_CONSTRUCT(class_name)              \
    DISABLE_MOVE_CONSTRUCT(class_name)

#endif
