#ifndef SYMTAB_H
#define SYMTAB_H

#include <lacc/array.h>
#include <lacc/hash.h>
#include <lacc/symbol.h>

/*
 * Delay initializing a new scope until new symbols are added.
 */
struct scope {
    struct hash_table table;
    enum {
        SCOPE_CREATED,      /* Pending hash_init. */
        SCOPE_DIRTY,        /* Pending hash_clear. */
        SCOPE_INITIALIZED   /* In use, containing symbols. */
    } state;
};

/*
 * A namespace holds symbols and manage resolution in scopes as they are
 * pushed or popped.
 */
struct namespace {
    /*
     * Friendly name of the namespace, can be identifier, label, tags or
     * anonymous.
     */
    const char *name;

    /*
     * All symbols, regardless of scope, are stored in the same list.
     * Must not be affected by reallocation, so store pointers.
     */
    array_of(struct symbol *) symbol;

    /*
     * Use hash table per scope, storing pointers to symbols in the
     * namespace symbol list.
     */
    array_of(struct scope) scope;

    /* Maximum number of scopes pushed. */
    int max_scope_depth;

    /* Iterator for successive calls to yield. */
    int cursor;
};

extern struct namespace
    ns_ident,   /* Identifiers. */
    ns_label,   /* Labels. */
    ns_tag;     /* Tags. */

void push_scope(struct namespace *ns);
void pop_scope(struct namespace *ns);

/*
 * Current depth as translation pushes and pops scopes. Depth 0 is
 * translation unit, 1 is function arguments, and n is local or member
 * variables.
 */
unsigned current_scope_depth(struct namespace *ns);

/*
 * Retrieve a symbol based on identifier name, or NULL of not registered
 * or visible from current scope.
 */
struct symbol *sym_lookup(struct namespace *ns, String name);

/*
 * Add symbol to current scope, or resolve to or complete existing
 * symbols when they occur repeatedly.
 */
struct symbol *sym_add(
    struct namespace *ns,
    String name,
    Type type,
    enum symtype symtype,
    enum linkage linkage);

/* Add symbol to current scope of given namespace. */
void sym_make_visible(struct namespace *ns, struct symbol *sym);

/*
 * Create a symbol with the provided type and add it to current scope in
 * identifier namespace. Used to hold temporary values in expression
 * evaluation.
 */
struct symbol *sym_create_temporary(Type type);

/*
 * Release memory used for a temporary symbol, allowing it to be reused
 * in a different function.
 */
void sym_release_temporary(struct symbol *sym);

/*
 * Retrieve next tentative definition or declaration from given scope.
 */
const struct symbol *yield_declaration(struct namespace *ns);

/* Verbose output all symbols from symbol table. */
void output_symbols(FILE *stream, struct namespace *ns);

#endif
