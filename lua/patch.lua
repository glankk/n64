console.write("applying patch...\n")
client.pause()
local patch = require("patch-data")
local patch_size = 0
for i = 1, #patch do
  local entry = patch[i]
  local address = entry[1]
  local data = entry[2]
  for j = 1, #data do
    mainmemory.writebyte(address, data[j])
    address = address + 1
    patch_size = patch_size + 1
  end
end
console.write(tostring(patch_size) .. " bytes written\n")
console.write("resetting dynrec...\n")
local state = memorysavestate.savecorestate()
memorysavestate.loadcorestate(state)
memorysavestate.removestate(state)
client.unpause()
console.write("done\n")
