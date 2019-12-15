#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define STACK_MAX 256
#define INITIAL_GC_THRESHOLD 8

typedef enum {
    OBJ_INT,
    OBJ_PAIR
} ObjectType;

typedef struct sObject {
    struct sObject* next;
    unsigned char marked;
    ObjectType objectType;

    union {
        int value;
        struct {
            struct sObject* head;
            struct sObject* tail;
        };
    };
} Object;

typedef struct {
    Object* firstObject;
    Object* stack[STACK_MAX];
    int stackSize;
    int numObjects;
    int maxObjects;
} VM;

VM* newVM() {
    VM* vm = malloc(sizeof(VM));
    vm->stackSize = 0;
    vm->firstObject = 0;
    vm->numObjects = 0;
    vm->maxObjects = INITIAL_GC_THRESHOLD;
    return vm;
}

void push(VM* vm, Object* object) {
    assert(vm->stackSize < STACK_MAX);
    vm->stack[vm->stackSize++] = object;
}

Object* pop(VM* vm) {
    assert(vm->stackSize > 0);
    Object* object = vm->stack[--vm->stackSize];
    return object;
}

Object* newObject(VM* vm, ObjectType objectType) {
    if (vm->numObjects == vm->maxObjects) { gc(vm); }

    Object* object = malloc(sizeof(Object));
    object->objectType = objectType;
    object->marked = 0;

    object->next = vm->firstObject;
    vm->numObjects++;
    vm->firstObject = object;
    return object;
} 

void pushInt(VM* vm, int i) {
    Object* object = newObject(vm, OBJ_INT);
    object->value = i;
    push(vm, object);
}

void pushPair(VM* vm) {
    Object* object = newObject(vm, OBJ_PAIR);
    object->head = pop(vm);
    object->tail = pop(vm);
    
    push(vm, object);
    return object;
}

void mark(Object* object) {
    if (object->marked) return;
    
    object->marked = 1;
    if (object->objectType == OBJ_PAIR) {
        mark(object->head);
        mark(object->tail);
    }
}

void markAll(VM* vm) {
    for (int i = 0; i < vm->stackSize; i++) {
        mark(vm->stack[i]);
    }
}

void sweep(VM* vm) {
    Object* object = vm->firstObject;
    while (object) {
        if (!object->marked) {
            Object* unreached = object;
            object = object->next;
            
            if (unreached == vm->firstObject) {
                vm->firstObject = object;
            }
            vm->numObjects--;
            free(unreached);
        } else {
            object->marked = 0;
            object = object->next;
        }
    }
}

void gc(VM* vm) {
    markAll(vm);
    sweep(vm);
    vm->maxObjects = vm->numObjects * 2;
}

void test1() {
  printf("Test 1: Objects on stack are preserved.\n");
  VM* vm = newVM();
  pushInt(vm, 1);
  pushInt(vm, 2);
     
  gc(vm);
  assert(vm->numObjects == 2);
}

void test2() {
  printf("Test 2: Unreached objects are collected.\n");
  VM* vm = newVM();
  pushInt(vm, 1);
  pushInt(vm, 2);
  pop(vm);
  pop(vm);

  gc(vm);
  assert(vm->numObjects == 0);
}


int main() {
    test1();
    test2();
    return 0;
}