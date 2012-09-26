#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>

#include <ruby.h>

double get_time(){
	struct timeval tv;
	gettimeofday( &tv, NULL );
	return tv.tv_sec + tv.tv_usec * 1e-6;
	}



/* Linked list  */

typedef struct LinkedList {
	char *key;
	void *value;
	struct LinkedList *next;
	} list_t;

void list_create( list_t **l, const char *key, void *value ){
	*l = (list_t*)malloc( sizeof( list_t ) );
	(*l)->key = (char *)malloc( strlen( key ) + 1 );
	strcpy( (*l)->key, key );
	(*l)->value = value;
	(*l)->next = NULL;
	}

void list_free( list_t *l ){
	if( l->next != NULL ){
		list_free( l->next );
		free( l->next );
		}
	}

void list_insert( list_t *l, const char *key, void *value ){
	list_t *current = l;
	while( current->next != NULL ){
		if( !strcmp( current->key, key ) ){
			free( current->value );
			current->value = value;
			return;
			}
		current = current->next;
		}
	list_create( &(current->next), key, value );
	}

void* list_fetch( list_t *l, const char *key ){	
	list_t *current = l;
	while( current != NULL ){
		if( !strcmp( current->key, key ) ){
			return current->value;
			}
		current = current->next;
		}
	return NULL;
	}

void list_remove( list_t *l, const char *key ){	
	list_t *current = l;
	list_t *prev = l;
	while( current != NULL ){
		if( !strcmp( current->key, key ) ){
			free( current->value );
			if( prev != current ){
				prev->next = current->next;
				}
			free( current );
			return;
			}
		prev = current;
		current = current->next;
		}
	}

void* list_pop_back( list_t **l ){	
	list_t *current = *l;
	list_t *prev = current;
	void *out;
	
	if( *l == NULL ){  return NULL; }

	while( current->next != NULL ){
		prev = current;
		current = current->next;
		}
	
	if( prev != current ){
		out = current->value;
		free( current );
		prev->next = NULL;
		}
	else {
		out = current->value;
		free( current );
		*l = NULL;
		}
	return out;
	}

/* Tries  */

typedef struct Trie64Map {
	uint8_t key;
	void *content;
	struct Trie64Map *leafs[16];
	} trie64_map_t;

void trie64_map_create( trie64_map_t **t, uint8_t key ){
	int i;

	*t = (trie64_map_t*)malloc( sizeof( trie64_map_t ) );
	(*t)->key = key;
	(*t)->content = NULL;
	for( i = 0 ; i < 16 ; ++i ){
		(*t)->leafs[i] = NULL;
		}
	}

void trie64_map_insert( trie64_map_t *t, uint64_t key, void *value ){
	uint8_t chunk;
	uint64_t i, shift, mask = 0x0F;
	trie64_map_t *current = t;
	for( i = 0 ; i < 16 ; ++i ){
		shift = 60 - i*4;
		chunk = ((key & (mask << shift ) ) >> shift) & mask;
		if( current->leafs[chunk] == NULL ){
			trie64_map_create( &(current->leafs[chunk] ), chunk );
			}
		current = current->leafs[chunk];
		}
	current->content = value;
	}

void* trie64_map_fetch( trie64_map_t *t, uint64_t key ){	
	uint8_t chunk, shift, mask = 0x0F;
	uint64_t i;
	trie64_map_t *current = t;
	for( i = 0 ; i < 16 ; ++i ){
		shift = 60 - i*4;
		chunk = ((key & (mask << shift ) ) >> shift) & mask;
		if( current->leafs[chunk] == NULL ){
			return NULL;	
			}	
		current = current->leafs[chunk];
		}
	return current->content;
	}


void trie64_map_remove( trie64_map_t *t, uint64_t key ){	
	uint8_t chunk, shift, mask = 0x0F;
	uint64_t i;
	trie64_map_t *current = t;
	for( i = 0 ; i < 15 ; ++i ){
		shift = 60 - i*4;
		chunk = ((key & (mask << shift ) ) >> shift) & mask;
		if( current->leafs[chunk] == NULL ){
			return;	
			}	
		current = current->leafs[chunk];
		}
	
	shift = 60 - i*4;
	chunk = ((key & (mask << shift ) ) >> shift) & mask;

	if( current->leafs[chunk] != NULL ){
		free( current->leafs[chunk] );
		current->leafs[chunk] = NULL;
		}
	}


void trie64_map_free( trie64_map_t *t ){
	int i;
	for( i = 0 ; i < 16 ; ++i ){
		if( t->leafs[i] != NULL ){
			trie64_map_free( t->leafs[i] );
			free( t->leafs[i] );
			}
		}
	}

void _trie64_map_get_all( trie64_map_t *t, uint64_t mask, uint64_t key, uint8_t level, list_t **l ){
	uint64_t i;
	uint64_t k, shift;
	shift = 60 - level*4;
	if( level < 16 ){
		for( i = 0 ; i < 16; ++i ){
			if( t->leafs[i] != NULL ){
				k = key | (i << shift);
				_trie64_map_get_all( t->leafs[i], mask, k, level + 1, l );
				}
			}
		}
	else{
		if( (mask & key) == mask ){
			char strkey[ sizeof( uint64_t ) + 1 ];
			memcpy( strkey, &key, sizeof( uint64_t ) );
			if( *l == NULL ){
				list_create( l, strkey, t->content );
				}
			else {
				list_insert( *l, strkey, t->content );
				}
			}
		}
	}

list_t* trie64_map_get_all( trie64_map_t *t, uint64_t mask ){
	list_t *l;
	list_create( &l, "", (void *)NULL );
	_trie64_map_get_all( t, mask, 0, 0, &l );
	return l;
	}


typedef struct Trie16Map {
	uint8_t key;
	void *content;
	struct Trie16Map *leafs[16];
	} trie16_map_t;

void trie16_map_create( trie16_map_t **t, uint8_t key ){
	int i;

	*t = (trie16_map_t*)malloc( sizeof( trie16_map_t ) );
	(*t)->key = key;
	(*t)->content = NULL;
	for( i = 0 ; i < 16 ; ++i ){
		(*t)->leafs[i] = NULL;
		}
	}


void trie16_map_insert( trie16_map_t *t, uint16_t key, void *value ){
	uint8_t chunk;
	uint16_t shift, mask = 0x0F;
	uint64_t i;
	trie16_map_t *current = t;
	for( i = 0 ; i < 4 ; ++i ){
		shift = 12 - i*4;
		chunk = ((key & (mask << shift ) ) >> shift) & mask;
		if( current->leafs[chunk] == NULL ){
			trie16_map_create( &(current->leafs[chunk] ), chunk );
			}
		current = current->leafs[chunk];
		}
	current->content = value;
	}

void* trie16_map_fetch( trie16_map_t *t, uint16_t key ){	
	uint8_t chunk;
	uint64_t shift, mask = 0x0F;
	uint64_t i;
	trie16_map_t *current = t;
	for( i = 0 ; i < 4 ; ++i ){
		shift = 12 - i*4;
		chunk = ((key & (mask << shift ) ) >> shift) & mask;
		if( current->leafs[chunk] == NULL ){
			return NULL;	
			}	
		current = current->leafs[chunk];
		}
	return current->content;
	}

void trie16_map_remove( trie16_map_t *t, uint16_t key ){	
	uint8_t chunk;
	uint64_t shift, mask = 0x0F;
	uint64_t i;
	trie16_map_t *current = t;
	for( i = 0 ; i < 3 ; ++i ){
		shift = 12 - i*4;
		chunk = ((key & (mask << shift ) ) >> shift) & mask;
		if( current->leafs[chunk] == NULL ){
			return;	
			}	
		current = current->leafs[chunk];
		}
	
	shift = 12 - i*4;
	chunk = ((key & (mask << shift ) ) >> shift) & mask;

	if( current->leafs[chunk] != NULL ){
		free( current->leafs[chunk] );
		current->leafs[chunk] = NULL;
		}
	}

void trie16_map_free( trie16_map_t *t ){
	int i;
	for( i = 0 ; i < 16 ; ++i ){
		if( t->leafs[i] != NULL ){
			trie16_map_free( t->leafs[i] );
			free( t->leafs[i] );
			}
		}
	}

typedef struct Trie64Set {
	uint8_t key;
	struct Trie64Set *leafs[16];
	} trie64_set_t;

void trie64_set_create( trie64_set_t **t, uint8_t key ){
	int i;

	*t = (trie64_set_t*)malloc( sizeof( trie64_set_t ) );
	(*t)->key = key;
	for( i = 0 ; i < 16 ; ++i ){
		(*t)->leafs[i] = NULL;
		}
	}


void trie64_set_insert( trie64_set_t *t, uint64_t key ){
	uint8_t chunk;
	uint64_t i, shift, mask = 0x0F;
	trie64_set_t *current = t;
	for( i = 0 ; i < 16 ; ++i ){
		shift = 60 - i*4;
		chunk = ((key & (mask << shift ) ) >> shift) & mask;
		if( current->leafs[chunk] == NULL ){
			trie64_set_create( &(current->leafs[chunk] ), chunk );
			}
		current = current->leafs[chunk];
		}
	}

int trie64_set_contains( trie64_set_t *t, uint64_t key ){	
	uint8_t chunk;
	uint64_t i, shift, mask = 0x0F;
	trie64_set_t *current = t;
	for( i = 0 ; i < 16 ; ++i ){
		shift = 60 - i*4;
		chunk = ((key & (mask << shift ) ) >> shift) & mask;
		if( current->leafs[chunk] == NULL ){
			return 0;	
			}	
		current = current->leafs[chunk];
		}
	return 1;
	}

void trie64_set_free( trie64_set_t *t ){
	int i;
	for( i = 0 ; i < 16 ; ++i ){
		if( t->leafs[i] != NULL ){
			trie64_set_free( t->leafs[i] );
			free( t->leafs[i] );
			}
		}
	}



typedef struct {
	uint64_t id;
	list_t *props;
	trie64_map_t *out, *in;
	} node_t;

typedef struct {
	uint64_t id;
	uint64_t source;
	uint64_t target;
	uint16_t type;
	double weight;
	} edge_t;

uint64_t edge_create_key( edge_t *e ){
	uint64_t out = 0;
	uint64_t shift = 48;
	out |= ((uint64_t)e->type) << shift;
	out |= e->target;
	return out;
	}

typedef struct {
	trie64_map_t *nodes;
	trie64_map_t *edges;
	uint64_t next_node_id, next_edge_id;
	} hawthorn_t;

void hawthorn_init( hawthorn_t **h ){
	(*h) = (hawthorn_t *)malloc( sizeof( hawthorn_t ) );
	(*h)->next_node_id = 1;
	(*h)->next_edge_id = 1;
	trie64_map_create( &(*h)->nodes, 0 );
	trie64_map_create( &(*h)->edges, 0 );
	}

uint64_t hawthorn_create_node( hawthorn_t *h ){
	uint64_t id = h->next_node_id;
	node_t *node;
	char *buffer;

	node = (node_t*)malloc( sizeof( node_t ) );
	node->id = id;
	buffer = (char *)malloc( 32 );
	sprintf( buffer, "%lx", (unsigned long int)node->id );


	list_create( &(node->props), "_id", (void*)buffer );
	trie64_map_create( &node->out, 0 );
	trie64_map_create( &node->in, 0 );

	trie64_map_insert( h->nodes, id, node );

	h->next_node_id += 1;
	return id;
	}

node_t* hawthorn_get_node( hawthorn_t *h, uint64_t id){
	return (node_t*)trie64_map_fetch( h->nodes, id );
	}

uint64_t hawthorn_connect( hawthorn_t *h, uint64_t source, uint64_t target, uint16_t type, double weight ){
	uint64_t id = h->next_edge_id;
	
	node_t *sn, *tn;
	edge_t *edge;

	sn = trie64_map_fetch( h->nodes, source );
	tn = trie64_map_fetch( h->nodes, target );

	if( sn == NULL || tn == NULL ){ return 0;  }

	edge = (edge_t*)malloc( sizeof( edge_t ) );
	
	edge->id = id;
	edge->source = source;
	edge->target = target;
	edge->type = type;
	edge->weight = weight;
	
	trie64_map_insert( sn->out, edge_create_key( edge ), edge );
	trie64_map_insert( tn->in, edge_create_key( edge ), edge );


	trie64_map_insert( h->edges, id, edge );

	h->next_edge_id += 1;
	return id;
	}

int hawthorn_disconnect( hawthorn_t *h, uint64_t source, uint64_t target, uint16_t type ){	
	node_t *sn, *tn;
	uint64_t edge_key;
	edge_t tmp, *edge;

	sn = trie64_map_fetch( h->nodes, source );
	tn = trie64_map_fetch( h->nodes, target );

	if( sn == NULL || tn == NULL ){ return 0;  }
	
	tmp.target = target;
	tmp.type = type;

	edge_key = edge_create_key( &tmp );

	edge = (edge_t*)trie64_map_fetch( sn->out, edge_key );
	/*printf( "disco: edge: %p \n", edge );*/
	if( edge != NULL ){
		/*printf( "edge->id: %lx, edge->source: %lx, edge->target: %lx \n", edge->id, edge->source, edge->target ); */
		trie64_map_remove( h->edges, edge->id );	
		trie64_map_remove( sn->out, edge_key );
		trie64_map_remove( tn->in, edge_key );

		free( edge );
		return 1;
		}
	return 0;
	}

list_t* hawthorn_get_outbound( hawthorn_t *db, uint64_t source, uint16_t type ){
	uint64_t mask;
	edge_t tmp;
	list_t *edges, *first;
	node_t *sn;
	
	tmp.type = type;
	tmp.target = 0;
	mask = edge_create_key( &tmp );

	sn = trie64_map_fetch( db->nodes, source );
	if( sn == NULL ){ return NULL;  }

	first = trie64_map_get_all( sn->out, mask );

	free( first );

	edges = first->next;
	return edges;
	}

list_t* hawthorn_get_inbound( hawthorn_t *db, uint64_t source, uint16_t type ){
	uint64_t mask;
	edge_t tmp;
	list_t *edges, *first;
	node_t *sn;
	
	tmp.type = type;
	tmp.target = 0;
	mask = edge_create_key( &tmp );

	sn = trie64_map_fetch( db->nodes, source );
	if( sn == NULL ){ return NULL;  }

	first = trie64_map_get_all( sn->in, mask );

	free( first );

	edges = first->next;
	return edges;
	}


list_t* hawthorn_get_connected( hawthorn_t *db, uint64_t source, uint16_t type ){
	trie64_set_t *visited;
	list_t *stack = NULL, *out;	
	char strkey[sizeof( uint64_t ) + 1];
	trie64_set_create( &visited, 0 );
	
	memcpy( strkey, &source, sizeof( uint64_t ) );
	list_create( &stack, strkey, (void*)source );
	list_create( &out, strkey, (void*)source );

	while( stack != NULL ){
		uint64_t node_id;
		void *tmp;
		list_t *nodes, *current;
		tmp = list_pop_back( &stack );
		if( tmp == NULL ){ continue; }
		node_id = (uint64_t)tmp;
		if( trie64_set_contains( visited, node_id ) ){
			continue;
			}
		
		trie64_set_insert( visited, node_id );

		nodes = hawthorn_get_outbound( db, node_id, type );
		if( nodes == NULL ){  continue;  }
		current = nodes;
		while( current != NULL ){
			edge_t *edge = (edge_t*)current->value;
			if( !trie64_set_contains( visited, edge->target ) ){
				memcpy( strkey, &(edge->target), sizeof( uint64_t ) );
				
				if( stack == NULL) {
					list_create( &stack, strkey, (void*)edge->target );
					}
				else {
					list_insert( stack, strkey, (void*)edge->target );
					}
				
				list_insert( out, strkey, (void*)edge->target );
				}
			current = current->next;
			}

		list_free( nodes );
		free( nodes );
		}
	trie64_set_free( visited );
	return out;
	}

int hawthorn_are_connected( hawthorn_t *db, uint64_t source, uint64_t target, uint16_t type ){
	trie64_set_t *visited;
	list_t *stack = NULL;	
	char strkey[sizeof( uint64_t ) + 1];
	trie64_set_create( &visited, 0 );
	
	memcpy( strkey, &source, sizeof( uint64_t ) );
	list_create( &stack, strkey, (void*)source );

	while( stack != NULL ){
		uint64_t node_id;
		void *tmp;
		list_t *nodes, *current;
		tmp = list_pop_back( &stack );
		if( tmp == NULL ){ continue; }
		node_id = (uint64_t)tmp;

		if( trie64_set_contains( visited, node_id ) ){
			continue;
			}
		
		trie64_set_insert( visited, node_id );

		nodes = hawthorn_get_outbound( db, node_id, type );
		if( nodes == NULL ){  continue;  }
		current = nodes;
		while( current != NULL ){
			edge_t *edge = (edge_t*)current->value;

			if( edge->target == target ){
				if( stack != NULL ){
					list_free( stack ); 
					free( stack );
					}
				list_free( nodes );
				free( nodes );
				trie64_set_free( visited );
				return 1;
				}

			if( !trie64_set_contains( visited, edge->target ) ){
				memcpy( strkey, &(edge->target), sizeof( uint64_t ) );
				
				if( stack == NULL) {
					list_create( &stack, strkey, (void*)edge->target );
					}
				else {
					list_insert( stack, strkey, (void*)edge->target );
					}
				
				}
			current = current->next;
			}

		list_free( nodes );
		free( nodes );
		}
	trie64_set_free( visited );
	return 0;
	}


hawthorn_t *databases[16];

static VALUE ht_Init( VALUE self, VALUE dbid ){
	int db_id = FIX2INT( dbid );

	if( databases[db_id] == NULL ){
		hawthorn_init( &databases[db_id] );
		}

	rb_iv_set( self, "@dbid", dbid );
	return self;
	}



static VALUE ht_create_node(  VALUE self ){
	VALUE dbid = rb_iv_get( self, "@dbid" );
	VALUE out;
	hawthorn_t *db = databases[ FIX2INT( dbid ) ];
	uint64_t node_id = hawthorn_create_node( db );
	out = INT2NUM( node_id );
	

	return out;
	}

static VALUE ht_connect_nodes( VALUE self, VALUE Rsource, VALUE Rtarget, VALUE Rtype, VALUE Rweight ){	
	VALUE dbid = rb_iv_get( self, "@dbid" );
	hawthorn_t *db = databases[ FIX2INT( dbid ) ];
	uint64_t source, target;
	uint16_t type;
	double weight;
	uint64_t retval;
	
	source = NUM2INT( Rsource );
	target = NUM2INT( Rtarget );
	type = FIX2INT( Rtype );
	weight = NUM2DBL( Rweight );

	retval = hawthorn_connect( db, source, target, type, weight );

	if( retval == 0 ){ return Qfalse;  }
	
	return Qtrue;	
	}

static VALUE ht_disconnect_nodes( VALUE self, VALUE Rsource, VALUE Rtarget, VALUE Rtype){	
	VALUE dbid = rb_iv_get( self, "@dbid" );
	hawthorn_t *db = databases[ FIX2INT( dbid ) ];
	uint64_t source, target;
	uint16_t type;
	int retval;

	source = NUM2INT( Rsource );
	target = NUM2INT( Rtarget );
	type = FIX2INT( Rtype );

	retval = hawthorn_disconnect( db, source, target, type );

	if( retval ){ return Qtrue;  }
	
	return Qfalse;	
	}

static VALUE ht_are_connected( VALUE self, VALUE Rsource, VALUE Rtarget, VALUE Rtype ){	
	VALUE dbid = rb_iv_get( self, "@dbid" );
	hawthorn_t *db = databases[ FIX2INT( dbid ) ];
	uint64_t source, target;
	uint16_t type;
	int retval;
	source = NUM2INT( Rsource );
	target = NUM2INT( Rtarget );
	type = FIX2INT( Rtype );
	
	retval = hawthorn_are_connected( db, source, target, type );
	

	if( retval ){ return Qtrue;  }
	
	return Qfalse;
	}

static VALUE ht_get_outbound( VALUE self, VALUE Rsource, VALUE Rtype ){	
	VALUE dbid = rb_iv_get( self, "@dbid" );
	hawthorn_t *db = databases[ FIX2INT( dbid ) ];
	list_t *edges, *current;
	uint64_t source;
	uint16_t type;
	VALUE out = rb_ary_new();

	source = NUM2INT( Rsource );
	type = FIX2INT( Rtype );
	edges = hawthorn_get_outbound( db, source, type );
	current = edges;
	while( current != NULL ){
		VALUE entry = rb_ary_new();
		VALUE e_source, e_target, e_type, e_weight;
		edge_t *edge = (edge_t*)(current->value);
		
		e_source = INT2NUM( edge->source );
		e_target = INT2NUM( edge->target );
		e_type = INT2FIX( edge->type );
		e_weight = rb_float_new( edge->weight );
		
		rb_ary_push( entry, e_source );
		rb_ary_push( entry, e_target );
		rb_ary_push( entry, e_type );
		rb_ary_push( entry, e_weight );
		
		rb_ary_push( out, entry );
		
		current = current->next;
		}

	list_free( edges );
	free( edges );
	return out;
	}

static VALUE ht_get_inbound( VALUE self, VALUE Rsource, VALUE Rtype ){	
	VALUE dbid = rb_iv_get( self, "@dbid" );
	hawthorn_t *db = databases[ FIX2INT( dbid ) ];
	list_t *edges, *current;
	uint64_t source;
	uint16_t type;
	VALUE out = rb_ary_new();

	source = NUM2INT( Rsource );
	type = FIX2INT( Rtype );
	edges = hawthorn_get_inbound( db, source, type );
	current = edges;
	while( current != NULL ){
		VALUE entry = rb_ary_new();
		VALUE e_source, e_target, e_type, e_weight;
		edge_t *edge = (edge_t*)(current->value);
		
		e_source = INT2NUM( edge->source );
		e_target = INT2NUM( edge->target );
		e_type = INT2FIX( edge->type );
		e_weight = rb_float_new( edge->weight );
		
		rb_ary_push( entry, e_source );
		rb_ary_push( entry, e_target );
		rb_ary_push( entry, e_type );
		rb_ary_push( entry, e_weight );
		
		rb_ary_push( out, entry );
		
		current = current->next;
		}

	list_free( edges );
	free( edges );
	return out;
	}


static VALUE ht_get_connected( VALUE self, VALUE Rsource, VALUE Rtype ){	
	VALUE dbid = rb_iv_get( self, "@dbid" );
	hawthorn_t *db = databases[ FIX2INT( dbid ) ];
	list_t *nodes, *current;
	uint64_t source;
	uint16_t type;
	VALUE out = rb_ary_new();

	source = NUM2INT( Rsource );
	type = FIX2INT( Rtype );
	nodes = hawthorn_get_connected( db, source, type );
	current = nodes;
	while( current != NULL ){
		VALUE node_id;
	
		node_id = INT2NUM( (uint64_t)(current->value) );

		rb_ary_push( out, node_id );
		
		current = current->next;
		}

	list_free( nodes );
	free( nodes );
	return out;
	}


static VALUE ht_get_properties( VALUE self, VALUE Rsource ){	
	VALUE dbid = rb_iv_get( self, "@dbid" );
	hawthorn_t *db = databases[ FIX2INT( dbid ) ];
	uint64_t node_id = NUM2INT( Rsource );
	VALUE out;
	list_t *current;
	node_t *node;
	node = hawthorn_get_node( db, node_id );
	if( node == NULL ){ return Qnil;  }
	
	out = rb_hash_new();

	current = node->props;
	while( current != NULL ){
		VALUE key, value;
		key = rb_str_new2( current->key );
		value = rb_str_new2( (char *)current->value );
		rb_hash_aset( out, key, value );
		current = current->next;
		}

	return out;
	}

static VALUE ht_set_property( VALUE self, VALUE Rsource, VALUE Rkey, VALUE Rvalue ){	
	VALUE dbid = rb_iv_get( self, "@dbid" );
	hawthorn_t *db = databases[ FIX2INT( dbid ) ];
	uint64_t node_id = NUM2INT( Rsource );
	node_t *node;
	char *value;
	node = hawthorn_get_node( db, node_id );
	if( node == NULL ){ return Qfalse;  }
	
	value = (char *)malloc( RSTRING_LEN(Rvalue) + 1 );

	memset( value, 0, RSTRING_LEN(Rvalue) + 1 );

	memcpy( value, RSTRING_PTR(Rvalue), RSTRING_LEN(Rvalue) );

	list_insert( node->props, RSTRING_PTR( Rkey ), value );


	return Qtrue;
	}


static VALUE ht_find( VALUE self, VALUE Rkey, VALUE Rvalue ){	
	VALUE dbid = rb_iv_get( self, "@dbid" );
	hawthorn_t *db = databases[ FIX2INT( dbid ) ];
	VALUE out;

	list_t *nodes, *current;
	nodes = trie64_map_get_all( db->nodes, 0 );
	current = nodes;
	out = rb_ary_new();

	while( current != NULL ){
		node_t *node = (node_t *)(current->value);
		void* value;
		if( node == NULL ){
			current = current->next;
			continue;
			}
		value = list_fetch( node->props, RSTRING_PTR( Rkey) );
		
		if( value != NULL ){
			uint64_t node_id;
			memcpy( &node_id, current->key, sizeof( uint64_t ) );
			if( !strcmp( (char *)value, RSTRING_PTR( Rvalue ) ) ){
				rb_ary_push( out, INT2NUM( node_id ) );
				}

			}
		current = current->next;
		}

	list_free( nodes );
	free( nodes );

	return out;
	}


static VALUE ht_all_nodes( VALUE self ){	
	VALUE dbid = rb_iv_get( self, "@dbid" );
	hawthorn_t *db = databases[ FIX2INT( dbid ) ];
	VALUE out;

	list_t *nodes, *current;
	nodes = trie64_map_get_all( db->nodes, 0 );
	current = nodes;
	out = rb_ary_new();

	while( current != NULL ){
		node_t *node = (node_t *)(current->value);
		void* value;
		if( node == NULL ){
			current = current->next;
			continue;
			}
		
		rb_ary_push( out, INT2NUM( node->id ) );

		current = current->next;
		}

	list_free( nodes );
	free( nodes );

	return out;
	}


void Init_hawthorn(void){
	VALUE klass = rb_define_class( "HawthornDB", rb_cObject );
	int arg_count = 1;

	rb_define_method( klass, "initialize", ht_Init, arg_count );

	arg_count = 0;
	rb_define_method( klass, "create_node", ht_create_node, arg_count );

	arg_count = 4;
	rb_define_method( klass, "connect", ht_connect_nodes, arg_count );

	arg_count = 3;
	rb_define_method( klass, "disconnected", ht_disconnect_nodes, arg_count );

	arg_count = 3;
	rb_define_method( klass, "are_connected", ht_are_connected, arg_count );
	
	arg_count = 2;
	rb_define_method( klass, "get_outbound", ht_get_outbound, arg_count );
	
	arg_count = 2;
	rb_define_method( klass, "get_inbound", ht_get_inbound, arg_count );
	
	arg_count = 2;
	rb_define_method( klass, "get_connected", ht_get_connected, arg_count );
	
	arg_count = 1;
	rb_define_method( klass, "get_properties", ht_get_properties, arg_count );
	
	arg_count = 3;
	rb_define_method( klass, "set_property", ht_set_property, arg_count );
	
	arg_count = 2;
	rb_define_method( klass, "find", ht_find, arg_count );
	
	arg_count = 0;
	rb_define_method( klass, "all_nodes", ht_all_nodes, arg_count );
	}
