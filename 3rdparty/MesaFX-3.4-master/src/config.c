/* $Id: config.c,v 1.10.4.1 2000/10/17 00:24:11 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  3.4
 * 
 * Copyright (C) 1999-2000  Brian Paul   All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


/* Mesa config file parse and execute code.
 * Copyright (C) 1999 Keith Whitwell.
 *
 * I hate parsers, so I've choosen a lisp-like syntax - extremely easy
 * to parse and potentially very expressive.  
 */


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "config.h"
#include "enums.h"
#include "extensions.h"
#include "hint.h"
#include "simple_list.h"
#include "mem.h"
#include "types.h"
#endif


typedef enum { nil_t, list_t, word_t } node_type;

struct cnode {
   node_type type;
   int line;
   union {
      struct { struct cnode *head, *tail; } l;
      struct { char *text; } w;
   } data;
};

/* Pretty printer for debugging.
 */
#if 0
static void pad(int n) { putchar('\n'); while(n--) putchar(' '); }

static void print_list( struct cnode *n, int indent ) 
{
   int i = 0;
   printf("( ");
   while (n->type == list_t) {
      if (i++ > 0) pad(indent + 2);
      switch (n->data.l.head->type) {
      case list_t:
	 print_list( n->data.l.head, indent + 2 );
	 break;
      case word_t:
	 printf( n->data.l.head->data.w.text );
	 break;
      case nil_t:
	 printf("()");
	 break;
      default:
	 puts("***");
      }
      n = n->data.l.tail;
   }
   printf(" )");
}
#endif

/* Accessors to query the contents of a cnode.
 */
static int is_list( struct cnode *x, struct cnode **h, struct cnode **t) 
{
   if (x->type == list_t) {
      struct cnode *tmp = x;
      *h = tmp->data.l.head;
      *t = tmp->data.l.tail;
      return 1;
   }
   return 0;
}

static int is_nil( const struct cnode *x ) 
{
   return x->type == nil_t;
}

static int is_word( struct cnode *x, const char **s ) 
{
   if (x->type == word_t) {
      *s = x->data.w.text;
      return 1;
   } 
   return 0;
}

static int match_word( struct cnode *x, const char *s ) 
{
   if (x->type == word_t) 
      return strcmp(s, x->data.w.text) == 0;
   return 0;
}

/* Build the parsed expression. 
 */
static void skip_comment( FILE *file )
{
   int c;
   while ((c = getc(file)) != EOF && c != '\n') {};
   ungetc( c, file );
}


static int cfg_isspace (int c)
{
   switch(c) {
   case ' ': case '\t':
   case '\n': case '\r':
   case '\f': case '\v': return 1;
   }
   return 0;
}

static struct cnode *get_word( int line, FILE *file )
{
   int sz = 16, len = 0;
   char *text = (char *) MALLOC( sz * sizeof(char) );
   
   while (1) {
      int c = getc(file);
      if (len == sz)  
	 text = (char *) realloc( text, sizeof(char) * (sz *= 2) );
      if (c == EOF || cfg_isspace(c) || c == ')') {
	 struct cnode *n = MALLOC_STRUCT(cnode);
	 ungetc(c, file);
	 text[len] = 0;
	 n->type = word_t;
	 n->line = line;
	 n->data.w.text = text;
	 return n;
      }   
      else
	 text[len++] = c;
   }
}

static struct cnode *get_list( int *line, FILE *file )
{
   struct cnode *head, **current = &head;
   head = MALLOC_STRUCT(cnode);
   head->line = *line;
   head->type = nil_t;

   while (1) {
      struct cnode *n = 0;
      int c = getc(file);

      switch (c) {
      case EOF: return head; 
      case ')': return head;
      case ';': skip_comment( file ); continue;
      case '\n': (*line)++; continue;
      case '(': 
	 n = get_list( line, file );
	 break;
      default: 
	 if (cfg_isspace(c)) continue;
	 ungetc(c, file); 
	 n = get_word( *line, file );
	 break;
      } 

      (*current)->type = list_t;
      (*current)->data.l.head = n;
      current = &(*current)->data.l.tail;
      (*current) = MALLOC_STRUCT(cnode);
      (*current)->line = *line;
      (*current)->type = nil_t;
   }
}

/* Execute it.
 */
static void error( struct cnode *n, const char *err )
{
   printf("Error in init file, line %d: %s\n", n->line, err );
}

static void disable_extension( GLcontext *ctx, struct cnode *args )
{
   struct cnode *head, *tail;
   const char *c;

   if (is_list(args, &head, &tail) && 
       is_nil(tail) &&
       is_word(head, &c)) 
   {
      if (gl_extensions_disable( ctx, c ) != 0)
	 error( head, "unknown extension" );
   }
   else
      error( args, "bad args for disable-extension" );	    
}


static void default_hint( GLcontext *ctx, struct cnode *args )
{
   struct cnode *hint, *tail, *value;
   const char *hname, *vname;

   if (is_list(args, &hint, &tail) && 
       is_list(tail, &value, &tail) &&
       is_nil(tail) &&
       is_word(hint, &hname) &&
       is_word(value, &vname))
   {
      GLint h = gl_lookup_enum_by_name(hname);
      GLint v = gl_lookup_enum_by_name(vname);
      if (h != -1 && v != -1)
      {
	 printf("calling glHint(%s=%d, %s=%d)\n", hname, h, vname, v);
	 if (!_mesa_try_Hint( ctx, (GLenum) h, (GLenum) v ))
	    error( hint, "glHint failed");
	 printf("allow draw mem: %d\n", ctx->Hint.AllowDrawMem);
	 return;
      }
      else
	 error( hint, "unknown or illegal value for default-hint" );
   }
   else
      error( args, "bad args for default-hint" );	    
}

/* Use the general-purpose set-variable 
 */
static void fx_catch_signals( GLcontext *ctx, struct cnode *args )
{
   struct cnode *head, *tail;
   const char *value;

/*     error( args,  */
/*  	  "fx-catch-signals deprecated, use " */
/*  	  "(set-variable fx-catch-signals ...) instead"); */

   if (is_list(args, &head, &tail) && 
       is_nil(tail) &&
       is_word(head, &value)) {
      if (strcmp(value, "false") == 0)
         ctx->CatchSignals = GL_FALSE;
      else if (strcmp(value, "true") == 0)
         ctx->CatchSignals = GL_TRUE;
      else
         error( args, "expected 'true' or 'false'" );
   }
   else {
      error( args, "bad args for fx-catch-signal" );
   }
}

/* Well, should these also check the environment?
 * Should environment vars override config vars?
 */

struct var {
   struct var *next, *prev;
   const char *name;
   void (*notify)(const char *value, int line);
};

static struct var varlist = { &varlist, &varlist, 0, 0 };

static void set_var( GLcontext *ctx, struct cnode *args )
{
   struct var *v;
   struct cnode *head, *tail;
   const char *variable, *value;

   if (is_list(args, &head, &tail) && 
       is_word(head, &variable) &&
       is_list(tail, &head, &tail) &&
       is_word(head, &value) &&
       is_nil(tail)) 
   {
      foreach(v, &varlist) {
	 if (strcmp(v->name, variable) == 0) {
	    v->notify(value, head->line);
	    return;
	 }
      }
      
      error( head, "unknown variable" );
   }
   else {
      error( args, "bad format for (set VARIABLE VALUE)" );
   }
}

void gl_register_config_var(const char *name, 
			    void (*notify)( const char *, int ))
{
   struct var *v = MALLOC_STRUCT(var);
   v->name = name;
   v->notify = notify;
   insert_at_tail( &varlist, v );
}


static void do_init( GLcontext *ctx, struct cnode *list )
{
   struct cnode *head, *tail, *func, *args;

   if (is_list(list, &head, &tail) && is_nil(tail)) {
      list = head;
      while (is_list(list, &head, &list)) {
	 if (is_list(head, &func, &args)) {
	    if (match_word(func, "disable-extension")) 
	       disable_extension( ctx, args );
	    else if (match_word(func, "default-hint")) 
	       default_hint( ctx, args );
	    else if (match_word(func, "fx-catch-signals")) 
	       fx_catch_signals( ctx, args );
            else if (match_word(func, "set-variable"))
               set_var( ctx, args );
	    else 
	       error( func, "unknown configuration method" );
         }
      }
   }
   else if (!is_nil(list)) {
      error( list, "configurations must form a list" );
   }
}

static int run_init( GLcontext *ctx, const char *version, struct cnode *list )
{
   struct cnode *head, *arg1, *arg2;
   const char *v;

   /* Uses the first matching init list.
    */
   while (is_list(list, &head, &list)) 
      if (is_list(head, &arg1, &head) &&
	  is_list(head, &arg2, &head) &&
	  match_word(arg1, "config-mesa") &&
	  is_word(arg2, &v))
      {
	 if (strcmp(v, version) == 0) {
	    do_init( ctx, head );
	    return 1;
	 }
      } 
      else
	 error( head, "malformed toplevel configuration" );

   return 0;
}



static void free_list( struct cnode *n ) 
{
   while (n->type == list_t) {
      struct cnode *tmp = n;      
      switch (n->data.l.head->type) {
      case list_t:
	 free_list( n->data.l.head );
	 break;
      case word_t:
	 FREE( n->data.l.head->data.w.text ); 
	 FREE( n->data.l.head );
	 break;
      case nil_t:
	 FREE( n->data.l.head );
	 break;	
      default:
	 return;
      }
      n = n->data.l.tail;
      FREE( tmp );
   }
   FREE( n );
}




/* How paranoid do you have to be when reading a config file?  I don't
 * know half the ways to exploit this stuff, and given that this may
 * be run with root access, I think we're better off hardcoding the
 * pathname.  Some clever joe can fix this later if they care.  
 */
void gl_read_config_file( GLcontext *ctx )
{
   const char *default_config = "mesa3.1beta1";

#if (defined(__WIN32__) && !defined(__CYGWIN__)) || defined(__MSDOS__)
   const char *filename = "mesa.cnf";
#else
   const char *filename = "/etc/mesa.conf"; 
#endif   
   FILE *file;
   struct cnode *list;
   int line = 1;
   char *v;

#if 0
   int f;
   struct stat statbuf;

   if ((f = open(filename, O_RDONLY)) == -1) 
      return;

   if (fstat( f, &statbuf ) == -1 || 
       !S_ISREG( statbuf.st_mode ) ||
       (file = fdopen(f, "r")) == 0) 
   {
      close( f );
      return;
   }
#endif

   if ((file = fopen(filename, "r")) == 0)
      return;

   list = get_list( &line, file );
   fclose( file );
   
   if ((v = getenv("MESA_CONFIG")) != 0 && *v != 0) {
      if (run_init( ctx, v, list )) {
	 free_list( list );
	 return;
      }
      else
	 fprintf(stderr, "No configuration '%s' in init file\n", v);
   }
	 

   if (!run_init( ctx, default_config, list )) {
      if (getenv("MESA_DEBUG")) {
         fprintf(stderr, "No default configuration '%s' in init file\n", 
                 default_config);
      }
   }

   free_list( list );
}


