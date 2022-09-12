
typedef struct blackboardS {
  uint8_t value[kMaxBlackboardItems][kMaxBlackboardValueSize];
} Blackboard;

typedef struct platform_eventS {
  // Type of event.
  int type;
  // Screen space the event took place in.
  int x;
  int y;
  // Event Detail
  union {
    float wheel_delta;
    char key;
    int button;
  };
} PlatformEvent;

typedef struct rectS {
  int x;
  int y;
  int w;
  int h;
} Rect;
