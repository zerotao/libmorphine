#!/usr/bin/env lua
require "dumper"

function dump(...)
	print(DataDumper(...), "\n---")
end

derp = {a="string", b={1,2,3}, c={a="b", b="c"}}

chno_raw = chno_from_lua(derp)
chno_lua = chno_to_lua(chno_raw)

dump(derp)
dump(chno_lua)

