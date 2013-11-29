#ifndef MUT_ACTION_H
#define MUT_ACTION_H

#include "mut_instr.h"		/* mut_instr */
#include "mut_counter.h"

enum mut_action_type {
	mut_action_type_malloc,
	mut_action_type_calloc,
	mut_action_type_realloc,
	mut_action_type_consumer,	/* covers free and cfree */
	mut_action_type_usage
};

typedef enum mut_action_type mut_action_type;

/*.dispatch: The preferred way of dispatching the action to be
 * performed is not based on a case statement, but is instead to embed a
 * pointer to the action function in the action ...
 *
 *  typedef struct mut_action *mut_action_rep;
 *
 *  typedef int (*mut_action_fun)(mut *, mut_exec_addr, mut_action_rep);
 *
 *  struct mut_action {
 *    char const        * name;
 *    mut_action_fun      exec;
 *    ...
 *  };
 *
 * Unfortunately, this means that the action is now dependent on mut.
 * Since a manager contains actions, this is a dependency we'd prefer to
 * avoid if possible.
 *
 * If this program was written in a language with some sort of
 * type-extension/inheritance, then a solution would be to define an
 * abstract manager type and make the action dependent on that.
 * That way the dependency between the concrete action and concrete
 * manager would be broken.  The C equivalent of this is to declare the
 * first argument in the above as a void * and do the necessary casting.
 *
 * If this program was written in a language with closures then it would
 * be simple to close over mut before calling a mut_action_fun which could
 * then access mut.
 *
 * If the program was written in a language which allowed nested classes
 * (e.g. Beta), then mut_action could be defined inside mut and so could
 * implicitly refer to mut.
 *
 * Since C doesn't support any of the above directly, the solution taken
 * here is to use tags and dispatch with a switch statement :-<
 *
 *
 *.map: All the actions need to be stored in a map with the key being
 * the action address.  Since there is only a small number of actions
 * then a simple linked list approach is used rather than having an
 * abstract interface which allows various different types of map to
 * be implemented.
 *
 *.instr: Each action contains an mut_instr which is used to hold the
 * instruction(s) that is overwritten by the breakpoint.
 */

struct mut_action {
	struct mut_action * next;	/* see .map */
	mut_exec_addr       addr;
	mut_instr           instr;
	mut_action_type     type;	/* see .dispatch */
	char const        * name;
	mut_counter         hits;
};

typedef struct mut_action mut_action;

#endif
