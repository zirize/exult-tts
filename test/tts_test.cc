// Runs tts.cc against test/fakecmd, which logs each line it is given instead of speaking.
// test/expected.log is what test/said.log must hold (`make check` diffs them): the first two
// lines are cut off (no "done" after them), the repeated book page is skipped.
#include "Configuration.h"
#include "tts.h"

#include <cstdio>
#include <string>
#include <unistd.h>

Configuration  c;
Configuration* config = &c;

int main() {
	std::remove("test/said.log");
	char cwd[4096];
	c.cmd = std::string(getcwd(cwd, sizeof cwd)) + "/test/fakecmd";
	TTS_init();
	TTS_say("@Greetings, Avatar!@~It is good to see thee.*How fare ye?*", "Iolo");
	usleep(300000);
	TTS_say("Second   page ^here.");
	usleep(300000);
	TTS_say("Book page", "", true);
	TTS_say("Book page", "", true);
	sleep(3);
	TTS_stop();
	return 0;
}
