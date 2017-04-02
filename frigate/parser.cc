/* A Bison parser, made by GNU Bison 2.7.12-4996.  */

/* Skeleton implementation for Bison LALR(1) parsers in C++
   
      Copyright (C) 2002-2013 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

// Take the name prefix into account.
#define yylex   compilerlex

/* First part of user declarations.  */
/* Line 283 of lalr1.cc  */
#line 5 "parser.yy"
 /*** C/C++ Declarations ***/

#include <stdio.h>
#include <string>
#include <vector>

#include "ast.h"


/* Line 283 of lalr1.cc  */
#line 50 "parser.cc"


#include "parser.hh"

/* User implementation prologue.  */
/* Line 289 of lalr1.cc  */
#line 152 "parser.yy"


#include "parse_driver.h"
#include "scanner.h"

/* this "connects" the bison parser in the driver to the flex scanner class
 * object. it defines the yylex() function call to pull the next token from the
 * current lexer object of the driver context. */
#undef yylex
#define yylex driver.lexer->lex

#define GETLOC AstLocation::ConvertLocation(yyloc.begin.line,yyloc.end.line,yyloc.begin.column,yyloc.end.column);


/* Line 289 of lalr1.cc  */
#line 73 "parser.cc"


# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* FIXME: INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (/*CONSTCOND*/ false)
# endif


/* Suppress unused-variable warnings by "using" E.  */
#define YYUSE(e) ((void) (e))

/* Enable debugging if requested.  */
#if YYDEBUG

/* A pseudo ostream that takes yydebug_ into account.  */
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)	\
do {							\
  if (yydebug_)						\
    {							\
      *yycdebug_ << Title << ' ';			\
      yy_symbol_print_ ((Type), (Value), (Location));	\
      *yycdebug_ << std::endl;				\
    }							\
} while (false)

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug_)				\
    yy_reduce_print_ (Rule);		\
} while (false)

# define YY_STACK_PRINT()		\
do {					\
  if (yydebug_)				\
    yystack_print_ ();			\
} while (false)

#else /* !YYDEBUG */

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Type, Value, Location) YYUSE(Type)
# define YY_REDUCE_PRINT(Rule)        static_cast<void>(0)
# define YY_STACK_PRINT()             static_cast<void>(0)

#endif /* !YYDEBUG */

#define yyerrok		(yyerrstatus_ = 0)
#define yyclearin	(yychar = yyempty_)

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)


namespace compiler {
/* Line 357 of lalr1.cc  */
#line 168 "parser.cc"

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  Parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr = "";
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              /* Fall through.  */
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }


  /// Build a parser object.
  Parser::Parser (class Driver& driver_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      driver (driver_yyarg)
  {
  }

  Parser::~Parser ()
  {
  }

#if YYDEBUG
  /*--------------------------------.
  | Print this symbol on YYOUTPUT.  |
  `--------------------------------*/

  inline void
  Parser::yy_symbol_value_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yyvaluep);
    std::ostream& yyo = debug_stream ();
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    YYUSE (yytype);
  }


  void
  Parser::yy_symbol_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    *yycdebug_ << (yytype < yyntokens_ ? "token" : "nterm")
	       << ' ' << yytname_[yytype] << " ("
	       << *yylocationp << ": ";
    yy_symbol_value_print_ (yytype, yyvaluep, yylocationp);
    *yycdebug_ << ')';
  }
#endif

  void
  Parser::yydestruct_ (const char* yymsg,
			   int yytype, semantic_type* yyvaluep, location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yymsg);
    YYUSE (yyvaluep);

    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

    YYUSE (yytype);
  }

  void
  Parser::yypop_ (unsigned int n)
  {
    yystate_stack_.pop (n);
    yysemantic_stack_.pop (n);
    yylocation_stack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  Parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  Parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  Parser::debug_level_type
  Parser::debug_level () const
  {
    return yydebug_;
  }

  void
  Parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif

  inline bool
  Parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  inline bool
  Parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  Parser::parse ()
  {
    /// Lookahead and lookahead in internal form.
    int yychar = yyempty_;
    int yytoken = 0;

    // State.
    int yyn;
    int yylen = 0;
    int yystate = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// Semantic value of the lookahead.
    static semantic_type yyval_default;
    semantic_type yylval = yyval_default;
    /// Location of the lookahead.
    location_type yylloc;
    /// The locations where the error started and ended.
    location_type yyerror_range[3];

    /// $$.
    semantic_type yyval;
    /// @$.
    location_type yyloc;

    int yyresult;

    // FIXME: This shoud be completely indented.  It is not yet to
    // avoid gratuitous conflicts when merging into the master branch.
    try
      {
    YYCDEBUG << "Starting parse" << std::endl;


/* User initialization code.  */
/* Line 539 of lalr1.cc  */
#line 44 "parser.yy"
{
    // initialize the initial location object
    yylloc.begin.filename = yylloc.end.filename = &driver.streamname;
}
/* Line 539 of lalr1.cc  */
#line 360 "parser.cc"

    /* Initialize the stacks.  The initial state will be pushed in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystate_stack_.clear ();
    yysemantic_stack_.clear ();
    yylocation_stack_.clear ();
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* New state.  */
  yynewstate:
    yystate_stack_.push (yystate);
    YYCDEBUG << "Entering state " << yystate << std::endl;

    /* Accept?  */
    if (yystate == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    /* Backup.  */
  yybackup:

    /* Try to take a decision without lookahead.  */
    yyn = yypact_[yystate];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    /* Read a lookahead token.  */
    if (yychar == yyempty_)
      {
        YYCDEBUG << "Reading a token: ";
        yychar = yylex (&yylval, &yylloc);
      }

    /* Convert token to internal form.  */
    if (yychar <= yyeof_)
      {
	yychar = yytoken = yyeof_;
	YYCDEBUG << "Now at end of input." << std::endl;
      }
    else
      {
	yytoken = yytranslate_ (yychar);
	YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
      }

    /* If the proper action on seeing token YYTOKEN is to reduce or to
       detect an error, take that action.  */
    yyn += yytoken;
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yytoken)
      goto yydefault;

    /* Reduce or error.  */
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
	if (yy_table_value_is_error_ (yyn))
	  goto yyerrlab;
	yyn = -yyn;
	goto yyreduce;
      }

    /* Shift the lookahead token.  */
    YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

    /* Discard the token being shifted.  */
    yychar = yyempty_;

    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* Count tokens shifted since error; after three, turn off error
       status.  */
    if (yyerrstatus_)
      --yyerrstatus_;

    yystate = yyn;
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystate];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    /* If YYLEN is nonzero, implement the default value of the action:
       `$$ = $1'.  Otherwise, use the top of the stack.

       Otherwise, the following line sets YYVAL to garbage.
       This behavior is undocumented and Bison
       users should not rely upon it.  */
    if (yylen)
      yyval = yysemantic_stack_[yylen - 1];
    else
      yyval = yysemantic_stack_[0];

    // Compute the default @$.
    {
      slice<location_type, location_stack_type> slice (yylocation_stack_, yylen);
      YYLLOC_DEFAULT (yyloc, slice, yylen);
    }

    // Perform the reduction.
    YY_REDUCE_PRINT (yyn);
    switch (yyn)
      {
          case 2:
/* Line 664 of lalr1.cc  */
#line 173 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node);}
    break;

  case 3:
/* Line 664 of lalr1.cc  */
#line 174 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node);}
    break;

  case 4:
/* Line 664 of lalr1.cc  */
#line 175 "parser.yy"
    {(yyval.node) = new ParenExprNode((yysemantic_stack_[(3) - (2)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 5:
/* Line 664 of lalr1.cc  */
#line 179 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node);}
    break;

  case 6:
/* Line 664 of lalr1.cc  */
#line 180 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node);}
    break;

  case 7:
/* Line 664 of lalr1.cc  */
#line 184 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 8:
/* Line 664 of lalr1.cc  */
#line 189 "parser.yy"
    {(yyval.node) = new DeclarationNode((yysemantic_stack_[(2) - (1)].node),(yysemantic_stack_[(2) - (2)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 9:
/* Line 664 of lalr1.cc  */
#line 190 "parser.yy"
    {(yyval.node) = new DeclarationNode((yysemantic_stack_[(1) - (1)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 10:
/* Line 664 of lalr1.cc  */
#line 194 "parser.yy"
    {(yyval.node) = new VarDeclarationCommaNode(); ((VarDeclarationCommaNode*)(yyval.node))->addNode((yysemantic_stack_[(1) - (1)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 11:
/* Line 664 of lalr1.cc  */
#line 195 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(3) - (1)].node); ((VarDeclarationCommaNode*)(yysemantic_stack_[(3) - (1)].node))->addNode((yysemantic_stack_[(3) - (3)].node));}
    break;

  case 12:
/* Line 664 of lalr1.cc  */
#line 199 "parser.yy"
    {(yyval.node) = new VarDeclarationNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (2)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 13:
/* Line 664 of lalr1.cc  */
#line 205 "parser.yy"
    {(yyval.node) = new VarDeclarationListNode(); ((VarDeclarationListNode*)(yyval.node))->addNode((yysemantic_stack_[(1) - (1)].node)); }
    break;

  case 14:
/* Line 664 of lalr1.cc  */
#line 206 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(2) - (1)].node); ((VarDeclarationListNode*)(yysemantic_stack_[(2) - (1)].node))->addNode((yysemantic_stack_[(2) - (2)].node));}
    break;

  case 15:
/* Line 664 of lalr1.cc  */
#line 212 "parser.yy"
    {(yyval.node) = new DefineNode((yysemantic_stack_[(3) - (2)].node),(yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 16:
/* Line 664 of lalr1.cc  */
#line 213 "parser.yy"
    {string s = *(yysemantic_stack_[(2) - (2)].stringVal); s = s.substr(1,s.length()-2);(yyval.node) = new IncludeNode(new TermNode(s)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 17:
/* Line 664 of lalr1.cc  */
#line 214 "parser.yy"
    {(yyval.node) = new OutputNode((yysemantic_stack_[(3) - (2)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 18:
/* Line 664 of lalr1.cc  */
#line 215 "parser.yy"
    {(yyval.node) = new InputNode((yysemantic_stack_[(3) - (2)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 19:
/* Line 664 of lalr1.cc  */
#line 216 "parser.yy"
    {(yyval.node) = new PartiesNode((yysemantic_stack_[(2) - (2)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 20:
/* Line 664 of lalr1.cc  */
#line 217 "parser.yy"
    {(yyval.node) = new StructTypedefNode((yysemantic_stack_[(6) - (3)].node),(yysemantic_stack_[(6) - (5)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 21:
/* Line 664 of lalr1.cc  */
#line 218 "parser.yy"
    {(yyval.node) = new IntTypedefNode((yysemantic_stack_[(4) - (4)].node),(yysemantic_stack_[(4) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 22:
/* Line 664 of lalr1.cc  */
#line 219 "parser.yy"
    {(yyval.node) = new UIntTypedefNode((yysemantic_stack_[(4) - (4)].node),(yysemantic_stack_[(4) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 23:
/* Line 664 of lalr1.cc  */
#line 224 "parser.yy"
    {(yyval.node) = new FunctionVarDeclarationNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (2)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 24:
/* Line 664 of lalr1.cc  */
#line 225 "parser.yy"
    {(yyval.node) = new FunctionVarDeclarationNode((yysemantic_stack_[(2) - (1)].node), (yysemantic_stack_[(2) - (2)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 25:
/* Line 664 of lalr1.cc  */
#line 231 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(3) - (1)].node); ((DeclarationArgCommaListNode*)(yysemantic_stack_[(3) - (1)].node))->addNode((yysemantic_stack_[(3) - (3)].node));}
    break;

  case 26:
/* Line 664 of lalr1.cc  */
#line 232 "parser.yy"
    {(yyval.node) = new DeclarationArgCommaListNode(); ((DeclarationArgCommaListNode*)(yyval.node))->addNode((yysemantic_stack_[(1) - (1)].node));}
    break;

  case 27:
/* Line 664 of lalr1.cc  */
#line 236 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node); }
    break;

  case 28:
/* Line 664 of lalr1.cc  */
#line 237 "parser.yy"
    {(yyval.node) = new AssignNode((yysemantic_stack_[(3) - (1)].node),(yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 29:
/* Line 664 of lalr1.cc  */
#line 253 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node);}
    break;

  case 30:
/* Line 664 of lalr1.cc  */
#line 254 "parser.yy"
    {(yyval.node) = new BitwiseORNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 31:
/* Line 664 of lalr1.cc  */
#line 258 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node);}
    break;

  case 32:
/* Line 664 of lalr1.cc  */
#line 259 "parser.yy"
    {(yyval.node) = new BitwiseXORNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 33:
/* Line 664 of lalr1.cc  */
#line 263 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node);}
    break;

  case 34:
/* Line 664 of lalr1.cc  */
#line 264 "parser.yy"
    {(yyval.node) = new BitwiseANDNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 35:
/* Line 664 of lalr1.cc  */
#line 268 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node);}
    break;

  case 36:
/* Line 664 of lalr1.cc  */
#line 269 "parser.yy"
    {(yyval.node) = new ConditionalEqualNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 37:
/* Line 664 of lalr1.cc  */
#line 270 "parser.yy"
    {(yyval.node) = new ConditionalNotEqualNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 38:
/* Line 664 of lalr1.cc  */
#line 275 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node);}
    break;

  case 39:
/* Line 664 of lalr1.cc  */
#line 276 "parser.yy"
    {(yyval.node) = new ConditionalLessNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 40:
/* Line 664 of lalr1.cc  */
#line 277 "parser.yy"
    {(yyval.node) = new ConditionalGreaterNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 41:
/* Line 664 of lalr1.cc  */
#line 278 "parser.yy"
    {(yyval.node) = new ConditionalLessEqualNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 42:
/* Line 664 of lalr1.cc  */
#line 279 "parser.yy"
    {(yyval.node) = new ConditionalGreaterEqualNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 43:
/* Line 664 of lalr1.cc  */
#line 283 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node);}
    break;

  case 44:
/* Line 664 of lalr1.cc  */
#line 284 "parser.yy"
    {(yyval.node) = new ShiftLeftNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 45:
/* Line 664 of lalr1.cc  */
#line 285 "parser.yy"
    {(yyval.node) = new ShiftRightNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 46:
/* Line 664 of lalr1.cc  */
#line 286 "parser.yy"
    {(yyval.node) = new RotateLeftNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 47:
/* Line 664 of lalr1.cc  */
#line 291 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node);}
    break;

  case 48:
/* Line 664 of lalr1.cc  */
#line 292 "parser.yy"
    {(yyval.node) = new ArithPlusNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 49:
/* Line 664 of lalr1.cc  */
#line 293 "parser.yy"
    {(yyval.node) = new ArithMinusNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 50:
/* Line 664 of lalr1.cc  */
#line 298 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node);}
    break;

  case 51:
/* Line 664 of lalr1.cc  */
#line 299 "parser.yy"
    {(yyval.node) = new ArithMultNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 52:
/* Line 664 of lalr1.cc  */
#line 300 "parser.yy"
    {(yyval.node) = new ArithDivNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 53:
/* Line 664 of lalr1.cc  */
#line 301 "parser.yy"
    {(yyval.node) = new ArithModuloNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 54:
/* Line 664 of lalr1.cc  */
#line 302 "parser.yy"
    {(yyval.node) = new ArithExMultNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 55:
/* Line 664 of lalr1.cc  */
#line 303 "parser.yy"
    {(yyval.node) = new ArithReDivNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 56:
/* Line 664 of lalr1.cc  */
#line 304 "parser.yy"
    {(yyval.node) = new ArithReModuloNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 57:
/* Line 664 of lalr1.cc  */
#line 309 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node);}
    break;

  case 58:
/* Line 664 of lalr1.cc  */
#line 314 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node);}
    break;

  case 59:
/* Line 664 of lalr1.cc  */
#line 315 "parser.yy"
    {(yyval.node) = new UnaryMinusNode((yysemantic_stack_[(2) - (2)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 60:
/* Line 664 of lalr1.cc  */
#line 316 "parser.yy"
    {(yyval.node) = new UnaryNOTNode((yysemantic_stack_[(2) - (2)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 61:
/* Line 664 of lalr1.cc  */
#line 317 "parser.yy"
    {(yyval.node) = new UnaryNOTNode((yysemantic_stack_[(2) - (2)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 62:
/* Line 664 of lalr1.cc  */
#line 318 "parser.yy"
    {(yyval.node) = new UnaryPrePlusPlusNode((yysemantic_stack_[(2) - (2)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 63:
/* Line 664 of lalr1.cc  */
#line 319 "parser.yy"
    {(yyval.node) = new UnaryPreMinusMinusNode((yysemantic_stack_[(2) - (2)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 64:
/* Line 664 of lalr1.cc  */
#line 323 "parser.yy"
    {(yyval.node) = new FunctionArgListNode(); ((FunctionArgListNode*)(yyval.node))->addNode((yysemantic_stack_[(1) - (1)].node));}
    break;

  case 65:
/* Line 664 of lalr1.cc  */
#line 324 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(3) - (1)].node); ((FunctionArgListNode*)(yysemantic_stack_[(3) - (1)].node))->addNode((yysemantic_stack_[(3) - (3)].node));}
    break;

  case 66:
/* Line 664 of lalr1.cc  */
#line 328 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node);}
    break;

  case 67:
/* Line 664 of lalr1.cc  */
#line 329 "parser.yy"
    {(yyval.node) = new WireAccessNode((yysemantic_stack_[(4) - (1)].node), (yysemantic_stack_[(4) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 68:
/* Line 664 of lalr1.cc  */
#line 330 "parser.yy"
    {(yyval.node) = new WireAccessNode((yysemantic_stack_[(6) - (1)].node), (yysemantic_stack_[(6) - (3)].node), (yysemantic_stack_[(6) - (5)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 69:
/* Line 664 of lalr1.cc  */
#line 334 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node);}
    break;

  case 70:
/* Line 664 of lalr1.cc  */
#line 335 "parser.yy"
    {(yyval.node) = new ArrayAccessNode((yysemantic_stack_[(4) - (1)].node), (yysemantic_stack_[(4) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 71:
/* Line 664 of lalr1.cc  */
#line 336 "parser.yy"
    {(yyval.node) = new FunctionCallNode((yysemantic_stack_[(3) - (1)].node),NULL); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 72:
/* Line 664 of lalr1.cc  */
#line 337 "parser.yy"
    {(yyval.node) = new FunctionCallNode((yysemantic_stack_[(4) - (1)].node),(yysemantic_stack_[(4) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 73:
/* Line 664 of lalr1.cc  */
#line 338 "parser.yy"
    {(yyval.node) = new DotOperatorNode((yysemantic_stack_[(3) - (1)].node),(yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 74:
/* Line 664 of lalr1.cc  */
#line 339 "parser.yy"
    {(yyval.node) = new UnaryPostPlusPlusNode((yysemantic_stack_[(2) - (1)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 75:
/* Line 664 of lalr1.cc  */
#line 340 "parser.yy"
    {(yyval.node) = new UnaryPostMinusMinusNode((yysemantic_stack_[(2) - (1)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 76:
/* Line 664 of lalr1.cc  */
#line 349 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node);}
    break;

  case 77:
/* Line 664 of lalr1.cc  */
#line 353 "parser.yy"
    {(yyval.node) = new ExpressionStatementNode((yysemantic_stack_[(2) - (1)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 78:
/* Line 664 of lalr1.cc  */
#line 354 "parser.yy"
    {(yyval.node) = new ExpressionStatementNode(0);}
    break;

  case 79:
/* Line 664 of lalr1.cc  */
#line 358 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node); }
    break;

  case 80:
/* Line 664 of lalr1.cc  */
#line 359 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node); }
    break;

  case 81:
/* Line 664 of lalr1.cc  */
#line 363 "parser.yy"
    {(yyval.node) = new ArrayInitListNode(); ((ArrayInitListNode*)(yyval.node))->addNode((yysemantic_stack_[(1) - (1)].node)); }
    break;

  case 82:
/* Line 664 of lalr1.cc  */
#line 364 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(3) - (1)].node); ((ArrayInitListNode*)(yysemantic_stack_[(3) - (1)].node))->addNode((yysemantic_stack_[(3) - (3)].node));}
    break;

  case 83:
/* Line 664 of lalr1.cc  */
#line 368 "parser.yy"
    {(yyval.node) = new ArrayInitNode((yysemantic_stack_[(3) - (2)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 84:
/* Line 664 of lalr1.cc  */
#line 373 "parser.yy"
    {(yyval.node) = new ArrayDecNode((yysemantic_stack_[(3) - (2)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 85:
/* Line 664 of lalr1.cc  */
#line 377 "parser.yy"
    {(yyval.node) = new ArrayDeclarationListNode(); ((ArrayDeclarationListNode*)(yyval.node))->addNode((yysemantic_stack_[(1) - (1)].node));}
    break;

  case 86:
/* Line 664 of lalr1.cc  */
#line 378 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(2) - (1)].node); ((ArrayDeclarationListNode*)(yysemantic_stack_[(2) - (1)].node))->addNode((yysemantic_stack_[(2) - (2)].node));}
    break;

  case 87:
/* Line 664 of lalr1.cc  */
#line 382 "parser.yy"
    {(yyval.node) = new VarDeclarationPrimNode((yysemantic_stack_[(2) - (1)].node),(yysemantic_stack_[(2) - (2)].node), NULL); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 88:
/* Line 664 of lalr1.cc  */
#line 383 "parser.yy"
    {(yyval.node) = new VarDeclarationPrimNode((yysemantic_stack_[(4) - (1)].node),(yysemantic_stack_[(4) - (2)].node), (yysemantic_stack_[(4) - (4)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 89:
/* Line 664 of lalr1.cc  */
#line 384 "parser.yy"
    {(yyval.node) = new VarDeclarationPrimNode((yysemantic_stack_[(1) - (1)].node),NULL, NULL); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 90:
/* Line 664 of lalr1.cc  */
#line 385 "parser.yy"
    {(yyval.node) = new VarDeclarationPrimNode((yysemantic_stack_[(3) - (1)].node),NULL,(yysemantic_stack_[(3) - (3)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 91:
/* Line 664 of lalr1.cc  */
#line 389 "parser.yy"
    {(yyval.node) = new DeclarationCommaListNode(); ((DeclarationCommaListNode*)(yyval.node))->addNode((yysemantic_stack_[(1) - (1)].node));}
    break;

  case 92:
/* Line 664 of lalr1.cc  */
#line 390 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(3) - (1)].node); ((DeclarationCommaListNode*)(yysemantic_stack_[(3) - (1)].node))->addNode((yysemantic_stack_[(3) - (3)].node));}
    break;

  case 93:
/* Line 664 of lalr1.cc  */
#line 394 "parser.yy"
    {(yyval.node) = new VarDeclarationNode((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (2)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 94:
/* Line 664 of lalr1.cc  */
#line 398 "parser.yy"
    {(yyval.node) = new VarDeclarationForNode((yysemantic_stack_[(2) - (1)].node), (yysemantic_stack_[(2) - (2)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 95:
/* Line 664 of lalr1.cc  */
#line 402 "parser.yy"
    {(yyval.node) = new ForNode((yysemantic_stack_[(9) - (3)].node),(yysemantic_stack_[(9) - (5)].node),(yysemantic_stack_[(9) - (7)].node),(yysemantic_stack_[(9) - (9)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 96:
/* Line 664 of lalr1.cc  */
#line 406 "parser.yy"
    {(yyval.node) = new IfNode((yysemantic_stack_[(7) - (3)].node),(yysemantic_stack_[(7) - (5)].node),(yysemantic_stack_[(7) - (7)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 97:
/* Line 664 of lalr1.cc  */
#line 407 "parser.yy"
    {(yyval.node) = new IfNode((yysemantic_stack_[(5) - (3)].node),(yysemantic_stack_[(5) - (5)].node),NULL); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 98:
/* Line 664 of lalr1.cc  */
#line 411 "parser.yy"
    {(yyval.node) = new ReturnNode((yysemantic_stack_[(3) - (2)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 99:
/* Line 664 of lalr1.cc  */
#line 412 "parser.yy"
    {(yyval.node) = new ReturnNode(NULL); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 100:
/* Line 664 of lalr1.cc  */
#line 416 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node);}
    break;

  case 101:
/* Line 664 of lalr1.cc  */
#line 417 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node);}
    break;

  case 102:
/* Line 664 of lalr1.cc  */
#line 418 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node);}
    break;

  case 103:
/* Line 664 of lalr1.cc  */
#line 419 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node);}
    break;

  case 104:
/* Line 664 of lalr1.cc  */
#line 420 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(1) - (1)].node);}
    break;

  case 105:
/* Line 664 of lalr1.cc  */
#line 425 "parser.yy"
    {(yyval.node) = new StatementListNode(); ((StatementListNode*)(yyval.node))->addNode((yysemantic_stack_[(1) - (1)].node)); }
    break;

  case 106:
/* Line 664 of lalr1.cc  */
#line 426 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(2) - (1)].node); ((StatementListNode*)(yysemantic_stack_[(2) - (1)].node))->addNode((yysemantic_stack_[(2) - (2)].node));}
    break;

  case 107:
/* Line 664 of lalr1.cc  */
#line 430 "parser.yy"
    {(yyval.node) = new CompoundStatementNode(NULL,false); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 108:
/* Line 664 of lalr1.cc  */
#line 431 "parser.yy"
    {(yyval.node) = new CompoundStatementNode((yysemantic_stack_[(3) - (2)].node),false); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 109:
/* Line 664 of lalr1.cc  */
#line 432 "parser.yy"
    {(yyval.node) = new CompoundStatementNode(NULL,true); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 110:
/* Line 664 of lalr1.cc  */
#line 433 "parser.yy"
    {(yyval.node) = new CompoundStatementNode((yysemantic_stack_[(3) - (2)].node),true); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 111:
/* Line 664 of lalr1.cc  */
#line 437 "parser.yy"
    {(yyval.node) = new CompoundStatementNode(NULL,false); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 112:
/* Line 664 of lalr1.cc  */
#line 438 "parser.yy"
    {(yyval.node) = new CompoundStatementNode((yysemantic_stack_[(3) - (2)].node),false); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 113:
/* Line 664 of lalr1.cc  */
#line 444 "parser.yy"
    {(yyval.node) = new StatementListNode(); ((StatementListNode*)(yyval.node))->addNode((yysemantic_stack_[(1) - (1)].node)); }
    break;

  case 114:
/* Line 664 of lalr1.cc  */
#line 445 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(2) - (1)].node); ((StatementListNode*)(yysemantic_stack_[(2) - (1)].node))->addNode((yysemantic_stack_[(2) - (2)].node));}
    break;

  case 115:
/* Line 664 of lalr1.cc  */
#line 449 "parser.yy"
    {(yyval.node) = new StatementListNode(); ((StatementListNode*)(yyval.node))->addNode((yysemantic_stack_[(1) - (1)].node)); }
    break;

  case 116:
/* Line 664 of lalr1.cc  */
#line 450 "parser.yy"
    {(yyval.node) = new StatementListNode(); ((StatementListNode*)(yyval.node))->addNode((yysemantic_stack_[(1) - (1)].node)); }
    break;

  case 117:
/* Line 664 of lalr1.cc  */
#line 451 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(2) - (1)].node); ((StatementListNode*)(yysemantic_stack_[(2) - (1)].node))->addNode((yysemantic_stack_[(2) - (2)].node));}
    break;

  case 118:
/* Line 664 of lalr1.cc  */
#line 452 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(2) - (1)].node); ((StatementListNode*)(yysemantic_stack_[(2) - (1)].node))->addNode((yysemantic_stack_[(2) - (2)].node));}
    break;

  case 119:
/* Line 664 of lalr1.cc  */
#line 457 "parser.yy"
    {(yyval.node) = new FunctionReturnTypeNode((yysemantic_stack_[(2) - (1)].node), (yysemantic_stack_[(2) - (2)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 120:
/* Line 664 of lalr1.cc  */
#line 458 "parser.yy"
    {(yyval.node) = new FunctionReturnTypeNode((yysemantic_stack_[(1) - (1)].node), 0); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 121:
/* Line 664 of lalr1.cc  */
#line 462 "parser.yy"
    {(yyval.node) = new FunctionDeclarationNode((yysemantic_stack_[(7) - (3)].node),(yysemantic_stack_[(7) - (5)].node),(yysemantic_stack_[(7) - (7)].node),(yysemantic_stack_[(7) - (2)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 122:
/* Line 664 of lalr1.cc  */
#line 463 "parser.yy"
    {(yyval.node) = new FunctionDeclarationNode((yysemantic_stack_[(6) - (3)].node),NULL,(yysemantic_stack_[(6) - (6)].node),(yysemantic_stack_[(6) - (2)].node)); (yyval.node)->nodeLocation = GETLOC;}
    break;

  case 123:
/* Line 664 of lalr1.cc  */
#line 466 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(2) - (1)].node); ((ProgramListNode*)(yysemantic_stack_[(2) - (1)].node))->addNode((yysemantic_stack_[(2) - (2)].node));}
    break;

  case 124:
/* Line 664 of lalr1.cc  */
#line 467 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(2) - (1)].node); ((ProgramListNode*)(yysemantic_stack_[(2) - (1)].node))->addNode((yysemantic_stack_[(2) - (2)].node));}
    break;

  case 125:
/* Line 664 of lalr1.cc  */
#line 468 "parser.yy"
    {(yyval.node) = (yysemantic_stack_[(2) - (1)].node); ((ProgramListNode*)(yysemantic_stack_[(2) - (1)].node))->addNode((yysemantic_stack_[(2) - (2)].node));}
    break;

  case 126:
/* Line 664 of lalr1.cc  */
#line 469 "parser.yy"
    {(yyval.node) = new ProgramListNode(); ((ProgramListNode*)(yyval.node))->addNode((yysemantic_stack_[(1) - (1)].node)); }
    break;

  case 127:
/* Line 664 of lalr1.cc  */
#line 470 "parser.yy"
    {(yyval.node) = new ProgramListNode(); ((ProgramListNode*)(yyval.node))->addNode((yysemantic_stack_[(1) - (1)].node));}
    break;

  case 128:
/* Line 664 of lalr1.cc  */
#line 475 "parser.yy"
    {AstLocation * loc = AstLocation::ConvertLocation(yyloc.begin.line,yyloc.end.line,yyloc.begin.column,yyloc.end.column);(yyval.node) = new TermNode(*(yysemantic_stack_[(1) - (1)].stringVal),loc);}
    break;

  case 129:
/* Line 664 of lalr1.cc  */
#line 479 "parser.yy"
    {AstLocation * loc = AstLocation::ConvertLocation(yyloc.begin.line,yyloc.end.line,yyloc.begin.column,yyloc.end.column);(yyval.node) = new TermNode(*(yysemantic_stack_[(1) - (1)].stringVal),1,loc);}
    break;

  case 130:
/* Line 664 of lalr1.cc  */
#line 480 "parser.yy"
    {AstLocation * loc = AstLocation::ConvertLocation(yyloc.begin.line,yyloc.end.line,yyloc.begin.column,yyloc.end.column);(yyval.node) = new TermNode(*(yysemantic_stack_[(3) - (1)].stringVal),1,loc); ((TermNode*)(yyval.node))->bitwidthnode = (yysemantic_stack_[(3) - (3)].node); (yysemantic_stack_[(3) - (3)].node)->parent = (yyval.node); ((TermNode*)(yyval.node))->isSigned=true;}
    break;

  case 131:
/* Line 664 of lalr1.cc  */
#line 481 "parser.yy"
    {AstLocation * loc = AstLocation::ConvertLocation(yyloc.begin.line,yyloc.end.line,yyloc.begin.column,yyloc.end.column);(yyval.node) = new TermNode(*(yysemantic_stack_[(3) - (1)].stringVal),1,loc); ((TermNode*)(yyval.node))->bitwidthnode = (yysemantic_stack_[(3) - (3)].node); (yysemantic_stack_[(3) - (3)].node)->parent = (yyval.node); ((TermNode*)(yyval.node))->isSigned=false; }
    break;

  case 133:
/* Line 664 of lalr1.cc  */
#line 485 "parser.yy"
    {driver.prog.topLevelNodes.push_back((yysemantic_stack_[(2) - (1)].node));}
    break;

  case 134:
/* Line 664 of lalr1.cc  */
#line 486 "parser.yy"
    {driver.prog.topLevelNodes.push_back((yysemantic_stack_[(2) - (1)].node));}
    break;


/* Line 664 of lalr1.cc  */
#line 1272 "parser.cc"
      default:
        break;
      }

    /* User semantic actions sometimes alter yychar, and that requires
       that yytoken be updated with the new translation.  We take the
       approach of translating immediately before every use of yytoken.
       One alternative is translating here after every semantic action,
       but that translation would be missed if the semantic action
       invokes YYABORT, YYACCEPT, or YYERROR immediately after altering
       yychar.  In the case of YYABORT or YYACCEPT, an incorrect
       destructor might then be invoked immediately.  In the case of
       YYERROR, subsequent parser actions might lead to an incorrect
       destructor call or verbose syntax error message before the
       lookahead is translated.  */
    YY_SYMBOL_PRINT ("-> $$ =", yyr1_[yyn], &yyval, &yyloc);

    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();

    yysemantic_stack_.push (yyval);
    yylocation_stack_.push (yyloc);

    /* Shift the result of the reduction.  */
    yyn = yyr1_[yyn];
    yystate = yypgoto_[yyn - yyntokens_] + yystate_stack_[0];
    if (0 <= yystate && yystate <= yylast_
	&& yycheck_[yystate] == yystate_stack_[0])
      yystate = yytable_[yystate];
    else
      yystate = yydefgoto_[yyn - yyntokens_];
    goto yynewstate;

  /*------------------------------------.
  | yyerrlab -- here on detecting error |
  `------------------------------------*/
  yyerrlab:
    /* Make sure we have latest lookahead translation.  See comments at
       user semantic actions for why this is necessary.  */
    yytoken = yytranslate_ (yychar);

    /* If not already recovering from an error, report this error.  */
    if (!yyerrstatus_)
      {
	++yynerrs_;
	if (yychar == yyempty_)
	  yytoken = yyempty_;
	error (yylloc, yysyntax_error_ (yystate, yytoken));
      }

    yyerror_range[1] = yylloc;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */
        if (yychar <= yyeof_)
          {
            /* Return failure if at end of input.  */
            if (yychar == yyeof_)
              YYABORT;
          }
        else
          {
            yydestruct_ ("Error: discarding", yytoken, &yylval, &yylloc);
            yychar = yyempty_;
          }
      }

    /* Else will try to reuse lookahead token after shifting the error
       token.  */
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if (false)
      goto yyerrorlab;

    yyerror_range[1] = yylocation_stack_[yylen - 1];
    /* Do not reclaim the symbols of the rule which action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    yystate = yystate_stack_[0];
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;	/* Each real token shifted decrements this.  */

    for (;;)
      {
	yyn = yypact_[yystate];
	if (!yy_pact_value_is_default_ (yyn))
	{
	  yyn += yyterror_;
	  if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
	    {
	      yyn = yytable_[yyn];
	      if (0 < yyn)
		break;
	    }
	}

	/* Pop the current state because it cannot handle the error token.  */
	if (yystate_stack_.height () == 1)
	  YYABORT;

	yyerror_range[1] = yylocation_stack_[0];
	yydestruct_ ("Error: popping",
		     yystos_[yystate],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);
	yypop_ ();
	yystate = yystate_stack_[0];
	YY_STACK_PRINT ();
      }

    yyerror_range[2] = yylloc;
    // Using YYLLOC is tempting, but would change the location of
    // the lookahead.  YYLOC is available though.
    YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yyloc);

    /* Shift the error token.  */
    YY_SYMBOL_PRINT ("Shifting", yystos_[yyn],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);

    yystate = yyn;
    goto yynewstate;

    /* Accept.  */
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    /* Abort.  */
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (yychar != yyempty_)
      {
        /* Make sure we have latest lookahead translation.  See comments
           at user semantic actions for why this is necessary.  */
        yytoken = yytranslate_ (yychar);
        yydestruct_ ("Cleanup: discarding lookahead", yytoken, &yylval,
                     &yylloc);
      }

    /* Do not reclaim the symbols of the rule which action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (1 < yystate_stack_.height ())
      {
        yydestruct_ ("Cleanup: popping",
                     yystos_[yystate_stack_[0]],
                     &yysemantic_stack_[0],
                     &yylocation_stack_[0]);
        yypop_ ();
      }

    return yyresult;
    }
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack"
                 << std::endl;
        // Do not try to display the values of the reclaimed symbols,
        // as their printer might throw an exception.
        if (yychar != yyempty_)
          {
            /* Make sure we have latest lookahead translation.  See
               comments at user semantic actions for why this is
               necessary.  */
            yytoken = yytranslate_ (yychar);
            yydestruct_ (YY_NULL, yytoken, &yylval, &yylloc);
          }

        while (1 < yystate_stack_.height ())
          {
            yydestruct_ (YY_NULL,
                         yystos_[yystate_stack_[0]],
                         &yysemantic_stack_[0],
                         &yylocation_stack_[0]);
            yypop_ ();
          }
        throw;
      }
  }

  // Generate an error message.
  std::string
  Parser::yysyntax_error_ (int yystate, int yytoken)
  {
    std::string yyres;
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    size_t yycount = 0;
    // Its maximum.
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    // Arguments of yyformat.
    char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yytoken) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yychar.
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state
         merging (from LALR or IELR) and default reductions corrupt the
         expected token list.  However, the list is correct for
         canonical LR with one exception: it will still contain any
         token that will not be accepted due to an error action in a
         later state.
    */
    if (yytoken != yyempty_)
      {
        yyarg[yycount++] = yytname_[yytoken];
        int yyn = yypact_[yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            /* Stay within bounds of both yycheck and yytname.  */
            int yychecklim = yylast_ - yyn + 1;
            int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
            for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
              if (yycheck_[yyx + yyn] == yyx && yyx != yyterror_
                  && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
                {
                  if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                    {
                      yycount = 1;
                      break;
                    }
                  else
                    yyarg[yycount++] = yytname_[yyx];
                }
          }
      }

    char const* yyformat = YY_NULL;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
        YYCASE_(0, YY_("syntax error"));
        YYCASE_(1, YY_("syntax error, unexpected %s"));
        YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    // Argument number.
    size_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += yytnamerr_ (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
  const short int Parser::yypact_ninf_ = -148;
  const short int
  Parser::yypact_[] =
  {
       128,    51,    10,    -1,    27,    -1,    -1,    10,  -148,  -148,
      65,    25,    10,   452,   452,  -148,   452,    30,    10,  -148,
    -148,    10,    10,    69,  -148,  -148,  -148,  -148,  -148,    10,
    -148,    15,   452,   452,   452,   452,   452,   452,  -148,  -148,
      48,    54,    52,    86,    67,    44,   111,   125,  -148,    60,
    -148,    26,    10,  -148,  -148,    10,  -148,   129,   129,  -148,
    -148,    91,   452,  -148,    69,  -148,   123,     0,    10,    93,
    -148,  -148,  -148,  -148,  -148,   452,   452,   452,   452,   452,
     452,   452,   452,   452,   452,   452,   452,   452,   452,   452,
     452,   452,   452,   452,   452,   452,   120,   452,    10,   452,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,    35,    97,
    -148,  -148,    10,   452,     1,  -148,    19,    10,  -148,    54,
    -148,    52,    86,    67,    67,    44,    44,    44,    44,   111,
     111,   111,   125,   125,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,    -3,    99,  -148,    45,    79,  -148,    55,
      10,  -148,  -148,  -148,   110,  -148,  -148,  -148,   137,    69,
    -148,   452,  -148,   452,  -148,   249,  -148,    79,    10,    69,
     420,  -148,  -148,    10,  -148,    69,  -148,   113,    21,  -148,
     310,  -148,   133,   136,   430,   142,  -148,  -148,  -148,  -148,
    -148,   138,  -148,   256,   139,    10,  -148,  -148,    69,  -148,
    -148,    16,  -148,  -148,  -148,  -148,  -148,   333,  -148,   365,
     452,    10,  -148,   152,  -148,  -148,   141,  -148,   420,  -148,
    -148,  -148,  -148,   160,   156,    10,  -148,  -148,   388,   452,
     157,   118,   168,   388,   452,  -148,   174,   388,  -148
  };

  /* YYDEFACT[S] -- default reduction number in state S.  Performed when
     YYTABLE doesn't specify something else to do.  Zero means the
     default is an error.  */
  const unsigned char
  Parser::yydefact_[] =
  {
       132,     0,     0,     0,     0,     0,     0,     0,   126,   127,
       0,     0,     0,     0,     0,   128,     0,   129,     0,    16,
      19,     0,     0,   120,   133,   134,   124,   125,   123,     0,
       1,     0,     0,     0,     0,     0,     0,     0,    69,    76,
      27,    29,    31,    33,    35,    38,    43,    47,    50,    57,
      58,    66,     0,     2,     3,     0,    15,     0,     0,    18,
      17,     0,     0,    85,   119,    91,     0,    89,     0,     0,
      60,    59,    61,    62,    63,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    21,    22,   130,     5,     6,   131,     0,     0,
      86,    93,     0,     0,    87,    13,     0,     0,     4,    30,
      57,    32,    34,    36,    37,    39,    40,    41,    42,    44,
      45,    46,    48,    49,    53,    56,    51,    54,    52,    55,
      28,    71,    64,     0,     0,    73,     0,     0,    26,     0,
       0,    84,    92,    90,     0,    20,    14,    10,     0,     9,
      72,     0,    70,     0,    67,     0,   122,     0,     0,    24,
       0,    88,    12,     0,     8,     7,    65,     0,     0,    78,
       0,   111,     0,     0,     0,     0,   100,   101,   103,   104,
     115,   113,   102,     0,     0,     2,   121,    25,    23,    79,
      81,     0,    80,    11,    68,   109,   105,     0,   107,     0,
       0,     0,    99,     0,    77,   117,   114,   112,     0,    83,
     110,   106,   108,     0,     0,     0,    98,    82,     0,     0,
      94,    97,     0,     0,     0,    96,     0,     0,    95
  };

  /* YYPGOTO[NTERM-NUM].  */
  const short int
  Parser::yypgoto_[] =
  {
      -148,  -148,   130,  -148,    14,  -148,    74,  -148,   182,    31,
    -148,   -84,  -148,   126,   127,   140,    83,    42,   -15,    77,
     121,   161,  -148,  -148,  -148,   -10,  -148,   -16,  -148,    50,
     -63,   -65,    94,    -9,   208,  -148,  -148,  -148,    28,  -147,
      40,  -148,    58,  -148,  -148,  -148,   212,  -148,    -2,    18,
    -148
  };

  /* YYDEFGOTO[NTERM-NUM].  */
  const short int
  Parser::yydefgoto_[] =
  {
        -1,    38,   104,   174,   157,   158,   115,   116,     8,   148,
     149,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,   143,    50,    51,   185,   186,   200,   201,   202,
      63,    64,    65,    66,   187,   224,   188,   189,   190,   206,
     207,   192,   166,   193,   194,    22,     9,    10,    53,    54,
      11
  };

  /* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule which
     number is the opposite.  If YYTABLE_NINF_, syntax error.  */
  const signed char Parser::yytable_ninf_ = -119;
  const short int
  Parser::yytable_[] =
  {
        16,   110,   114,    52,    55,    23,    56,    17,    29,   160,
      31,   140,   142,    62,    62,   161,    59,    15,   191,    60,
      61,    18,    69,    20,    21,    30,    15,    67,    15,    17,
     178,   205,    32,    19,   218,   113,   154,    96,   179,    97,
      33,    98,    15,    68,    34,   219,   216,   147,   155,   180,
     102,   110,   109,   103,    99,   105,   105,    35,    36,    37,
     221,   163,   221,   100,   101,    24,   117,   167,    25,   129,
     130,   131,    15,   168,   164,   106,   106,   176,   182,   183,
      75,   231,    62,    84,    85,    77,   235,   144,    76,   146,
     238,    86,    57,    58,   175,    95,   145,    80,    81,    12,
      13,    14,   108,   153,   198,   118,   150,   165,    82,    83,
      67,   151,   110,   162,   117,   159,     1,     2,     3,     4,
       5,     6,   125,   126,   127,   128,     7,    15,    17,    78,
      79,    32,   141,    87,    88,   110,    15,    17,   170,    33,
     111,   112,   204,    34,   210,    89,    90,   211,   169,    91,
      92,    93,    94,   177,   172,   173,    35,    36,    37,   214,
     199,   123,   124,   195,   132,   133,   150,  -116,   217,   226,
    -118,   159,   228,   229,   213,   112,   195,   233,   195,     1,
       2,     3,     4,     5,     6,   234,   237,   203,   107,     7,
     156,   195,    26,    67,    70,    71,    72,    73,    74,   197,
     223,   119,   227,   121,   171,   195,   152,   195,   199,   225,
     134,   135,   136,   137,   138,   139,   230,   122,    27,   232,
     209,   215,    28,    67,   236,   196,   195,     0,     0,     0,
       0,   195,     0,     0,     0,   195,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,    15,    17,   178,     0,
      32,     0,     0,    15,    17,   178,   179,    32,    33,     0,
       0,     0,    34,   179,     0,    33,     0,   180,   181,    34,
       0,     0,     0,     0,   180,    35,    36,    37,     0,     0,
       0,     0,    35,    36,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   182,   183,     0,   184,
       0,     0,     0,   182,   183,     0,   184,    15,    17,   178,
       0,    32,     0,     0,     0,     0,     0,   179,     0,    33,
       0,     0,     0,    34,     0,     0,     0,     0,   180,   208,
      15,    17,   178,   220,    32,     0,    35,    36,    37,     0,
     179,     0,    33,     0,     0,     0,    34,     0,     0,     0,
       0,   180,     0,     0,     0,     0,     0,   182,   183,    35,
      36,    37,    15,    17,   178,     0,    32,     0,     0,     0,
       0,     0,   179,     0,    33,     0,     0,     0,    34,     0,
     182,   183,     0,   180,   222,    15,    17,   178,     0,    32,
       0,    35,    36,    37,     0,   179,     0,    33,     0,     0,
       0,    34,     0,     0,     0,     0,   180,     0,     0,     0,
       0,     0,   182,   183,    35,    36,    37,    15,    17,     0,
       0,    32,     0,     0,     0,     0,     0,    15,    17,    33,
       0,    32,     0,    34,     0,   182,   183,   212,   170,    33,
       0,     0,     0,    34,     0,     0,    35,    36,    37,    15,
      17,     0,     0,    32,     0,     0,    35,    36,    37,     0,
       0,    33,     0,     0,     0,    34,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    35,    36,
      37
  };

  /* YYCHECK.  */
  const short int
  Parser::yycheck_[] =
  {
         2,    64,    67,    13,    14,     7,    16,     8,    10,    12,
      12,    95,    96,    13,    13,    18,    18,     7,   165,    21,
      22,     3,    32,     5,     6,     0,     7,    29,     7,     8,
       9,    10,    11,     6,    18,    35,    35,    11,    17,    13,
      19,    15,     7,    28,    23,    29,   193,    12,    29,    28,
      52,   114,    62,    55,    28,    57,    58,    36,    37,    38,
     207,    16,   209,    37,    38,     0,    68,    12,     3,    84,
      85,    86,     7,    18,    29,    57,    58,   161,    57,    58,
      32,   228,    13,    39,    40,    33,   233,    97,    34,    99,
     237,    47,    62,    63,   159,    35,    98,    30,    31,    48,
      49,    50,    11,   113,   169,    12,   108,    28,    41,    42,
     112,    14,   175,    14,   116,   117,    51,    52,    53,    54,
      55,    56,    80,    81,    82,    83,    61,     7,     8,    43,
      44,    11,    12,    22,    23,   198,     7,     8,    28,    19,
      17,    18,    29,    23,    11,    20,    21,    11,   150,    24,
      25,    26,    27,   163,    17,    18,    36,    37,    38,    17,
     170,    78,    79,   165,    87,    88,   168,    29,    29,    17,
      29,   173,    12,    17,   184,    18,   178,    59,   180,    51,
      52,    53,    54,    55,    56,    17,    12,   173,    58,    61,
     116,   193,    10,   195,    33,    34,    35,    36,    37,   168,
     210,    75,   218,    76,   154,   207,   112,   209,   218,   211,
      89,    90,    91,    92,    93,    94,   225,    77,    10,   229,
     180,   193,    10,   225,   234,   167,   228,    -1,    -1,    -1,
      -1,   233,    -1,    -1,    -1,   237,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     7,     8,     9,    -1,
      11,    -1,    -1,     7,     8,     9,    17,    11,    19,    -1,
      -1,    -1,    23,    17,    -1,    19,    -1,    28,    29,    23,
      -1,    -1,    -1,    -1,    28,    36,    37,    38,    -1,    -1,
      -1,    -1,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    58,    -1,    60,
      -1,    -1,    -1,    57,    58,    -1,    60,     7,     8,     9,
      -1,    11,    -1,    -1,    -1,    -1,    -1,    17,    -1,    19,
      -1,    -1,    -1,    23,    -1,    -1,    -1,    -1,    28,    29,
       7,     8,     9,    10,    11,    -1,    36,    37,    38,    -1,
      17,    -1,    19,    -1,    -1,    -1,    23,    -1,    -1,    -1,
      -1,    28,    -1,    -1,    -1,    -1,    -1,    57,    58,    36,
      37,    38,     7,     8,     9,    -1,    11,    -1,    -1,    -1,
      -1,    -1,    17,    -1,    19,    -1,    -1,    -1,    23,    -1,
      57,    58,    -1,    28,    29,     7,     8,     9,    -1,    11,
      -1,    36,    37,    38,    -1,    17,    -1,    19,    -1,    -1,
      -1,    23,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,
      -1,    -1,    57,    58,    36,    37,    38,     7,     8,    -1,
      -1,    11,    -1,    -1,    -1,    -1,    -1,     7,     8,    19,
      -1,    11,    -1,    23,    -1,    57,    58,    17,    28,    19,
      -1,    -1,    -1,    23,    -1,    -1,    36,    37,    38,     7,
       8,    -1,    -1,    11,    -1,    -1,    36,    37,    38,    -1,
      -1,    19,    -1,    -1,    -1,    23,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    37,
      38
  };

  /* STOS_[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
  const unsigned char
  Parser::yystos_[] =
  {
         0,    51,    52,    53,    54,    55,    56,    61,    72,   110,
     111,   114,    48,    49,    50,     7,   112,     8,   113,     6,
     113,   113,   109,   112,     0,     3,    72,    98,   110,   112,
       0,   112,    11,    19,    23,    36,    37,    38,    65,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      87,    88,    89,   112,   113,    89,    89,    62,    63,   112,
     112,   112,    13,    94,    95,    96,    97,   112,    28,    89,
      85,    85,    85,    85,    85,    32,    34,    33,    43,    44,
      30,    31,    41,    42,    39,    40,    47,    22,    23,    20,
      21,    24,    25,    26,    27,    35,    11,    13,    15,    28,
      37,    38,   112,   112,    66,   112,   113,    66,    11,    89,
      94,    17,    18,    35,    95,    70,    71,   112,    12,    77,
      85,    78,    79,    80,    80,    81,    81,    81,    81,    82,
      82,    82,    83,    83,    84,    84,    84,    84,    84,    84,
      75,    12,    75,    86,    89,   112,    89,    12,    73,    74,
     112,    14,    96,    89,    35,    29,    70,    68,    69,   112,
      12,    18,    14,    16,    29,    28,   106,    12,    18,   112,
      28,    93,    17,    18,    67,    95,    75,    89,     9,    17,
      28,    29,    57,    58,    60,    89,    90,    98,   100,   101,
     102,   103,   105,   107,   108,   112,   106,    73,    95,    89,
      91,    92,    93,    68,    29,    10,   103,   104,    29,   104,
      11,    11,    17,    89,    17,   102,   103,    29,    18,    29,
      10,   103,    29,    89,    99,   112,    17,    91,    12,    17,
      97,   103,    89,    59,    17,   103,    89,    12,   103
  };

#if YYDEBUG
  /* TOKEN_NUMBER_[YYLEX-NUM] -- Internal symbol number corresponding
     to YYLEX-NUM.  */
  const unsigned short int
  Parser::yytoken_number_[] =
  {
         0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318
  };
#endif

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
  const unsigned char
  Parser::yyr1_[] =
  {
         0,    64,    65,    65,    65,    66,    66,    67,    68,    68,
      69,    69,    70,    71,    71,    72,    72,    72,    72,    72,
      72,    72,    72,    73,    73,    74,    74,    75,    75,    76,
      76,    77,    77,    78,    78,    79,    79,    79,    80,    80,
      80,    80,    80,    81,    81,    81,    81,    82,    82,    82,
      83,    83,    83,    83,    83,    83,    83,    84,    85,    85,
      85,    85,    85,    85,    86,    86,    87,    87,    87,    88,
      88,    88,    88,    88,    88,    88,    89,    90,    90,    91,
      91,    92,    92,    93,    94,    95,    95,    96,    96,    96,
      96,    97,    97,    98,    99,   100,   101,   101,   102,   102,
     103,   103,   103,   103,   103,   104,   104,   105,   105,   105,
     105,   106,   106,   107,   107,   108,   108,   108,   108,   109,
     109,   110,   110,   111,   111,   111,   111,   111,   112,   113,
     113,   113,   114,   114,   114
  };

  /* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
  const unsigned char
  Parser::yyr2_[] =
  {
         0,     2,     1,     1,     3,     1,     1,     1,     2,     1,
       1,     3,     3,     1,     2,     3,     2,     3,     3,     2,
       6,     4,     4,     3,     2,     3,     1,     1,     3,     1,
       3,     1,     3,     1,     3,     1,     3,     3,     1,     3,
       3,     3,     3,     1,     3,     3,     3,     1,     3,     3,
       1,     3,     3,     3,     3,     3,     3,     1,     1,     2,
       2,     2,     2,     2,     1,     3,     1,     4,     6,     1,
       4,     3,     4,     3,     2,     2,     1,     2,     1,     1,
       1,     1,     3,     3,     3,     1,     2,     2,     4,     1,
       3,     1,     3,     3,     2,     9,     7,     5,     3,     2,
       1,     1,     1,     1,     1,     1,     2,     2,     3,     2,
       3,     2,     3,     1,     2,     1,     1,     2,     2,     2,
       1,     7,     6,     2,     2,     2,     1,     1,     1,     1,
       3,     3,     0,     2,     2
  };


  /* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
     First, the terminals, then, starting at \a yyntokens_, nonterminals.  */
  const char*
  const Parser::yytname_[] =
  {
    "\"end of file\"", "error", "$undefined", "\"end of line\"",
  "\"integer\"", "\"double\"", "\"STRING\"", "\"term\"", "\"number\"",
  "\"+{\"", "\"}+\"", "\"(\"", "\")\"", "\"[\"", "\"]\"", "\".\"", "\":\"",
  "\";\"", "\",\"", "\"!\"", "\"%\"", "\"%%\"", "\"+\"", "\"-\"", "\"*\"",
  "\"**\"", "\"/\"", "\"//\"", "\"{\"", "\"}\"", "\"<\"", "\">\"", "\"|\"",
  "\"&\"", "\"^\"", "\"=\"", "\"~\"", "\"++\"", "\"--\"", "\"<<\"",
  "\">>\"", "\"<=\"", "\">=\"", "\"==\"", "\"!=\"", "\"&&\"", "\"||\"",
  "\"<<>\"", "\"struct_t\"", "\"int_t\"", "\"uint_t\"", "\"typedef\"",
  "\"#define\"", "\"#input\"", "\"#include\"", "\"#parties\"",
  "\"#output\"", "\"if\"", "\"for\"", "\"else\"", "\"return\"",
  "\"function\"", "\"#\"", "\"#u\"", "$accept", "primary_expression",
  "term_expression", "postdec", "struct_prim_var_dec",
  "struct_high_var_dec", "struct_vardec", "struct_vardeclist",
  "preamble_rule", "defarg", "defarglist", "assignment_expression",
  "inclusive_or_expression", "exclusive_or_expression", "and_expression",
  "equality_expression", "relational_expression", "shift_expression",
  "additive_expression", "multiplicative_expression", "cast_expression",
  "unary_expression", "argument_expression_list", "wire_expression",
  "postfix_expression", "expression", "expression_statement",
  "array_init_operand", "array_init_list", "array_init", "array_dec",
  "array_dec_list", "var_dec_prim", "var_dec_right_side",
  "var_dec_statement", "var_dec_for_statement", "for_statement",
  "if_statement", "return_statement", "statement", "statement_list",
  "compound_statement", "function_compound_statement",
  "end_function_statement_list", "start_function_statement_list",
  "return_type", "function_def", "program_node", "term_rule",
  "term_num_rule", "start", YY_NULL
  };

#if YYDEBUG
  /* YYRHS -- A `-1'-separated list of the rules' RHS.  */
  const Parser::rhs_number_type
  Parser::yyrhs_[] =
  {
       114,     0,    -1,   112,    -1,   113,    -1,    11,    89,    12,
      -1,   112,    -1,   113,    -1,    95,    -1,   112,    67,    -1,
     112,    -1,    68,    -1,    69,    18,    68,    -1,   112,    69,
      17,    -1,    70,    -1,    71,    70,    -1,    52,   112,    89,
      -1,    54,     6,    -1,    56,   113,   112,    -1,    53,   113,
     112,    -1,    55,   113,    -1,    51,    48,   112,    28,    71,
      29,    -1,    51,    49,    89,   112,    -1,    51,    50,    89,
     112,    -1,   112,   112,    95,    -1,   112,   112,    -1,    74,
      18,    73,    -1,    73,    -1,    76,    -1,    85,    35,    75,
      -1,    77,    -1,    76,    32,    77,    -1,    78,    -1,    77,
      34,    78,    -1,    79,    -1,    78,    33,    79,    -1,    80,
      -1,    79,    43,    80,    -1,    79,    44,    80,    -1,    81,
      -1,    80,    30,    81,    -1,    80,    31,    81,    -1,    80,
      41,    81,    -1,    80,    42,    81,    -1,    82,    -1,    81,
      39,    82,    -1,    81,    40,    82,    -1,    81,    47,    82,
      -1,    83,    -1,    82,    22,    83,    -1,    82,    23,    83,
      -1,    84,    -1,    83,    24,    84,    -1,    83,    26,    84,
      -1,    83,    20,    84,    -1,    83,    25,    84,    -1,    83,
      27,    84,    -1,    83,    21,    84,    -1,    85,    -1,    87,
      -1,    23,    85,    -1,    19,    85,    -1,    36,    85,    -1,
      37,    85,    -1,    38,    85,    -1,    75,    -1,    86,    18,
      75,    -1,    88,    -1,    88,    28,    89,    29,    -1,    88,
      28,    89,    16,    89,    29,    -1,    65,    -1,    88,    13,
      89,    14,    -1,    88,    11,    12,    -1,    88,    11,    86,
      12,    -1,    88,    15,   112,    -1,    88,    37,    -1,    88,
      38,    -1,    75,    -1,    89,    17,    -1,    17,    -1,    89,
      -1,    93,    -1,    91,    -1,    92,    18,    91,    -1,    28,
      92,    29,    -1,    13,    89,    14,    -1,    94,    -1,    95,
      94,    -1,   112,    95,    -1,   112,    95,    35,    93,    -1,
     112,    -1,   112,    35,    89,    -1,    96,    -1,    97,    18,
      96,    -1,   112,    97,    17,    -1,   112,    97,    -1,    58,
      11,    99,    17,    89,    17,    89,    12,   103,    -1,    57,
      11,    89,    12,   103,    59,   103,    -1,    57,    11,    89,
      12,   103,    -1,    60,    89,    17,    -1,    60,    17,    -1,
      90,    -1,    98,    -1,   105,    -1,   100,    -1,   101,    -1,
     103,    -1,   104,   103,    -1,    28,    29,    -1,    28,   104,
      29,    -1,     9,    10,    -1,     9,   104,    10,    -1,    28,
      29,    -1,    28,   108,    29,    -1,   103,    -1,   107,   103,
      -1,   102,    -1,   103,    -1,   107,   102,    -1,   107,   103,
      -1,   112,    95,    -1,   112,    -1,    61,   109,   112,    11,
      74,    12,   106,    -1,    61,   109,   112,    11,    12,   106,
      -1,   111,   110,    -1,   111,    72,    -1,   111,    98,    -1,
      72,    -1,   110,    -1,     7,    -1,     8,    -1,     8,    62,
      66,    -1,     8,    63,    66,    -1,    -1,   111,     0,    -1,
     111,     3,    -1
  };

  /* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
     YYRHS.  */
  const unsigned short int
  Parser::yyprhs_[] =
  {
         0,     0,     3,     5,     7,    11,    13,    15,    17,    20,
      22,    24,    28,    32,    34,    37,    41,    44,    48,    52,
      55,    62,    67,    72,    76,    79,    83,    85,    87,    91,
      93,    97,    99,   103,   105,   109,   111,   115,   119,   121,
     125,   129,   133,   137,   139,   143,   147,   151,   153,   157,
     161,   163,   167,   171,   175,   179,   183,   187,   189,   191,
     194,   197,   200,   203,   206,   208,   212,   214,   219,   226,
     228,   233,   237,   242,   246,   249,   252,   254,   257,   259,
     261,   263,   265,   269,   273,   277,   279,   282,   285,   290,
     292,   296,   298,   302,   306,   309,   319,   327,   333,   337,
     340,   342,   344,   346,   348,   350,   352,   355,   358,   362,
     365,   369,   372,   376,   378,   381,   383,   385,   388,   391,
     394,   396,   404,   411,   414,   417,   420,   422,   424,   426,
     428,   432,   436,   437,   440
  };

  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
  const unsigned short int
  Parser::yyrline_[] =
  {
         0,   173,   173,   174,   175,   179,   180,   184,   189,   190,
     194,   195,   199,   205,   206,   212,   213,   214,   215,   216,
     217,   218,   219,   224,   225,   231,   232,   236,   237,   253,
     254,   258,   259,   263,   264,   268,   269,   270,   275,   276,
     277,   278,   279,   283,   284,   285,   286,   291,   292,   293,
     298,   299,   300,   301,   302,   303,   304,   309,   314,   315,
     316,   317,   318,   319,   323,   324,   328,   329,   330,   334,
     335,   336,   337,   338,   339,   340,   349,   353,   354,   358,
     359,   363,   364,   368,   373,   377,   378,   382,   383,   384,
     385,   389,   390,   394,   398,   402,   406,   407,   411,   412,
     416,   417,   418,   419,   420,   425,   426,   430,   431,   432,
     433,   437,   438,   444,   445,   449,   450,   451,   452,   457,
     458,   462,   463,   466,   467,   468,   469,   470,   475,   479,
     480,   481,   484,   485,   486
  };

  // Print the state stack on the debug stream.
  void
  Parser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (state_stack_type::const_iterator i = yystate_stack_.begin ();
	 i != yystate_stack_.end (); ++i)
      *yycdebug_ << ' ' << *i;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  Parser::yy_reduce_print_ (int yyrule)
  {
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    /* Print the symbols being reduced, and their result.  */
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
	       << " (line " << yylno << "):" << std::endl;
    /* The symbols being reduced.  */
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
		       yyrhs_[yyprhs_[yyrule] + yyi],
		       &(yysemantic_stack_[(yynrhs) - (yyi + 1)]),
		       &(yylocation_stack_[(yynrhs) - (yyi + 1)]));
  }
#endif // YYDEBUG

  /* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
  Parser::token_number_type
  Parser::yytranslate_ (int t)
  {
    static
    const token_number_type
    translate_table[] =
    {
           0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63
    };
    if ((unsigned int) t <= yyuser_token_number_max_)
      return translate_table[t];
    else
      return yyundef_token_;
  }

  const int Parser::yyeof_ = 0;
  const int Parser::yylast_ = 490;
  const int Parser::yynnts_ = 51;
  const int Parser::yyempty_ = -2;
  const int Parser::yyfinal_ = 30;
  const int Parser::yyterror_ = 1;
  const int Parser::yyerrcode_ = 256;
  const int Parser::yyntokens_ = 64;

  const unsigned int Parser::yyuser_token_number_max_ = 318;
  const Parser::token_number_type Parser::yyundef_token_ = 2;


} // compiler
/* Line 1135 of lalr1.cc  */
#line 2079 "parser.cc"
/* Line 1136 of lalr1.cc  */
#line 491 "parser.yy"
 /*** Additional Code ***/

void compiler::Parser::error(const Parser::location_type& l,
			    const std::string& m)
{
    driver.error(l, m);
exit(0);
}
