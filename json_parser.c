#include "sit.h"

#define MAX_DEPTH 10

typedef struct JSONState {
  jsonsl_t    json_parser;
  Parser     *tokenizer;
  DocBuf     *buffers[MAX_DEPTH];
  pstring    *working;      // the live window we care about
  long        working_pos;  // the offset of working relative to the stream start
  pstring    *delta;        // the string passed with consume
} JSONState;

int 
_jsonsl_error_callback(
        jsonsl_t jsn,
        jsonsl_error_t error,
        struct jsonsl_state_st* state,
        jsonsl_char_t *at) {
  (void) state;
  (void) at;
	Parser *parser = jsn->data;
  parser->receiver->error_found(parser->receiver, c2pstring(jsonsl_strerror(error)));
	return 0;
}

void 
_jsonsl_stack_callback(
        jsonsl_t jsn,
        jsonsl_action_t action,
        struct jsonsl_state_st* state,
        const jsonsl_char_t *at) {
	Parser *parser = jsn->data;
	JSONState *mystate = parser->state;
  pstring *working = mystate->working;
  long working_pos = mystate->working_pos;
  Receiver *buf = &mystate->buffers[state->level]->as_receiver;
	switch (action) {
	case JSONSL_ACTION_POP: 
		switch (state->type) {
	  case JSONSL_T_SPECIAL: {
  	  if(!(state->special_flags & JSONSL_SPECIALf_NUMERIC)) {
        break;
      }
      const char *string = working->val + state->pos_begin - working_pos;
      buf->int_found(buf, strtol(string, NULL, 10));
      break;
    }
		case JSONSL_T_HKEY: {
      long off = state->pos_cur - working_pos + 1;
      int len = state->pos_cur - state->pos_begin - 2;
      pstring pstr = {
        working->val + off,
        len
      };
      buf->field_found(buf, &pstr);
			break;
		}
		case JSONSL_T_STRING: {
  		long off = state->pos_cur - working_pos + 1;
      int len = state->pos_cur - state->pos_begin - 2;
      pstring pstr = {
        working->val + off,
        len
      };

      mystate->tokenizer->data = parser->data;
			mystate->tokenizer->receiver = parser->receiver;
      mystate->tokenizer->consume(mystate->tokenizer, &pstr);
      mystate->tokenizer->end_stream(mystate->tokenizer);

			break;
    }
		case JSONSL_T_OBJECT: {
     	long off = state->pos_cur - working_pos;
      int len = state->pos_cur - state->pos_begin;
      pstring doc = {
        working->val + off,
        len
      };
		  buf->document_found(parser->receiver, &doc);
      break;
		}}
		break;
	case JSONSL_ACTION_ERROR: 
		break;
	}
}

Parser *
json_white_parser_new() {
  return json_parser_new(white_parser_new());
}

Parser *
json_fresh_copy(Parser *parser) {
	JSONState *state = parser->state;
	return json_parser_new(state->tokenizer->fresh_copy(state->tokenizer));
}

void 
_json_consume(struct Parser *parser, pstring *str) {
  assert(parser->receiver);
  JSONState *state = parser->state;
  padd(state->working, str);
  state->delta = str;
	jsonsl_feed(state->json_parser, str->val, str->len);
}

Parser *
json_parser_new(Parser *tokenizer) {
  Parser *parser = parser_new();
  parser->state = malloc(sizeof(JSONState));
  JSONState *state = parser->state;
  state->json_parser = jsonsl_new(MAX_DEPTH);
  jsonsl_enable_all_callbacks(state->json_parser);
  jsonsl_reset(state->json_parser);
  state->tokenizer = tokenizer;
  state->json_parser->action_callback = _jsonsl_stack_callback;
	state->json_parser->error_callback = _jsonsl_error_callback;
	SET_ONCE(state->json_parser->data, parser);
	parser->fresh_copy = json_fresh_copy;
  parser->consume = _json_consume;
  state->working = pstring_new(0);
  state->working_pos = 0;
  for(int i =0; i < MAX_DEPTH; i++) {
    state->buffers[i] = doc_buf_new(10000);
  }
  return parser;
}
