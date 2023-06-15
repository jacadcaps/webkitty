# Copyright (C) 2011 Google Inc. All rights reserved.
# Copyright (C) 2020 Apple Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#    * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#    * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import logging
import unittest

from webkitpy.tool.mocktool import MockOptions, MockTool
from webkitpy.tool.steps.applywatchlist import ApplyWatchList

from webkitcorepy import OutputCapture


class ApplyWatchListTest(unittest.TestCase):
    def test_apply_watch_list_local(self):
        step = ApplyWatchList(MockTool(log_executive=True), MockOptions())
        state = {
            'bug_id': '50001',
            'diff': 'The diff',
        }
        with OutputCapture(level=logging.INFO) as captured:
            step.run(state)
        self.assertEqual(
            captured.root.log.getvalue(),
            '''MockWatchList: determine_cc_and_messages
MOCK bug comment: bug_id=50001, cc=['levin@chromium.org'], see_also=None
--- Begin comment ---
Message2.
--- End comment ---

Result of watchlist: cc "levin@chromium.org" messages "Message2."
''',
        )
