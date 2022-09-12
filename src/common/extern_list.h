
/*
// in common
djb2_hash_more(unsigned char const*, unsigned int, unsigned long*)
common_getopt(int, char* const*, char const*)
// (log.cc)
  PopLogMessage()
  PushLogMessage(LogMessage)
  CountLogMessage()
  Log(char const*, unsigned int)
  ReadLog(int)
  LogCount()

// clock
clock_init(unsigned long, TscClock_t*)
clock_sync(TscClock_t*, unsigned long*)
clock_delta_usec(TscClock_t const*)
__init_tsc_per_usec()

// network
// platform_udpS; init(), receive(), sendto(), replace poll?
//
udp::ReceiveAny(Udp4, unsigned short, unsigned char*, unsigned short*, Udp4*)
udp::ReceiveFrom(Udp4, unsigned short, unsigned char*, short*)
udp::SetLowDelay(Udp4)
udp::Bind(Udp4)
udp::Init()
udp::Send(Udp4, void const*, unsigned short)
udp::SendTo(Udp4, Udp4, void const*, unsigned short)
udp::BindAddr(Udp4, char const*, char const*)
udp::GetAddr4(char const*, char const*, Udp4*)
udp::PollUsec(Udp4, unsigned long)

// window
// platform_windowS init(), update()
window::ShouldClose()
window::SwapBuffers()
window::GetWindowSize(int*, int*)
window::x11_error_handler(_XDisplay*, XErrorEvent*)
window::x11_ioerror_handler(_XDisplay*)
window::Create(char const*, int, int, bool)
window::Create(char const*, window::CreateInfo const&)

// input
// platform_inputS poll()
window::GetCursorPosition(int*, int*) // hardware cursor??
window::PollEvent(PlatformEvent*)

// scheduler
platform::sleep_usec(unsigned long)

// threading
// platform_threadS create, yield, sleep, join, exit
platform::thread_exit(ThreadInfo*, unsigned long)
platform::thread_join(ThreadInfo*)
platform::pthread_shim(void*)
platform::thread_yield()
platform::thread_create(ThreadInfo*)
platform::thread_affinity_count()
platform::thread_affinity_usecore(int)
platform::thread_affinity_avoidcore(int)
*/
