
#include <am.h>
#include <klib.h>
#include <rtthread.h>

typedef struct CallContext {
  void *tentry;
  void *parameter;
  void *texit;
} CallContext_t;

static void kthread(void *arg) {
  CallContext_t * ctx = arg;
  ((void (*)(void *))(ctx->tentry))(ctx->parameter); //转换为函数指针
  ((void (*)(void))(ctx->texit))();
  while (1) {
    ;
  }
}

static Context *ev_handler(Event e, Context *c) {
  switch (e.event) {
  case EVENT_YIELD:
    rt_thread_t current = rt_thread_self();
    rt_ubase_t *data = (rt_ubase_t *)current->user_data;
    rt_ubase_t to = data[1];
    rt_ubase_t from = data[0];
    if (from != 0) {
      *((Context **)from) = c;
    }
    c = *((Context **)to);
    break;
  case EVENT_IRQ_TIMER:
    break;
  default:
    printf("Unhandled event ID = %d\n", e.event);
    assert(0);
  }
  return c;
}

void __am_cte_init(void) { 
  cte_init(ev_handler); 
}
/*
to和from都是指向上下文指针变量的指针(二级指针). rt_hw_context_switch_to()用于切换到to指向的上下文指针
变量所指向的上下文, 而rt_hw_context_switch()还需要额外将当前上下文的指针写入from指向的上下文指针变量中. 
为了进行切换, 我们可以通过yield()触发一次自陷, 在事件处理回调函数ev_handler()中识别出EVENT_YIELD事件后, 
再处理to和from. 同样地, 我们需要思考如何将to和from这两个参数传给ev_handler()
*/
void rt_hw_context_switch_to(rt_ubase_t to) {
  rt_ubase_t temp;
  rt_ubase_t data[2];
  rt_thread_t current = rt_thread_self();
  temp = current->user_data;
  data[0] = 0;
  data[1] = to;
  current->user_data = (rt_ubase_t)data;
  yield();
  current->user_data = temp;
}

void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to) {
  rt_ubase_t temp;
  rt_ubase_t data[2];
  rt_thread_t current = rt_thread_self();
  temp = current->user_data;
  data[0] = from;
  data[1] = to;
  current->user_data = (rt_ubase_t)data;
  yield();
  current->user_data = temp;
}



void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from, rt_ubase_t to, struct rt_thread *to_thread) {
  assert(0);
}
/*
以stack_addr为栈底创建一个入口为tentry, 参数为parameter的上下文, 并返回这个上下文结构的指针. 
此外, 若上下文对应的内核线程从tentry返回, 则调用texit, RT-Thread会保证代码不会从texit中返回
*/
/*
CTE的kcontext()要求不能从入口返回, 因此需要一种新的方式来支持texit的功能. 一种方式是构造一个包
裹函数, 让包裹函数来调用tentry, 并在tentry返回后调用texit, 然后将这个包裹函数作为kcontext()的真正入口
*/
rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter, rt_uint8_t *stack_addr, void *texit) {
  uintptr_t aligned_addr = (uintptr_t)stack_addr;
  aligned_addr &= ~(sizeof(uintptr_t) - 1); // 向下对齐到 sizeof(uintptr_t)
  rt_uint8_t *sp = (rt_uint8_t *)aligned_addr;
  sp -= sizeof(CallContext_t);

  CallContext_t * callCtx = (CallContext_t *)sp;
  *callCtx = (CallContext_t){
      .tentry = tentry,
      .parameter = parameter,
      .texit = texit,
  };

  Context * ctx = kcontext((Area){.start = NULL, .end = sp}, kthread, callCtx);

  return (rt_uint8_t *)ctx;
}
