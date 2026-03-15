#pragma once

enum Opcode : int {
  UPDATE_MAP = 1,
};

struct MapOp {
  Opcode opcode; // operation code : 1 - update value
  int arg1;      // customer_id to apply the operation
  int arg2;      // parameter for the operation
};