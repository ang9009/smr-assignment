#pragma once

struct MapOp {
  int opcode; // operation code : 1 - update value
  int arg1;   // customer_id to apply the operation
  int arg2;   // parameter for the operation
};