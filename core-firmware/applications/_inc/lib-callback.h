#ifndef Callback_h
#define Callback_h

class Callback
{
    public:
      Callback(void);
      void (*callback)(void*);
      void execute();
      void* context;
      int set;
};

Callback::Callback(void) {
    set = 0;
}

void Callback::execute() {
    (*callback)(context);
}

#endif