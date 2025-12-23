# Scenario: READ - Cache Hit
lui   t0, 0x1           # t0 = 0x1000
addi  t1, zero, 123     # t1 = 123
sw    t1, 0(t0)         # write 123 to 0x1000 (writes to memory, not cache depending on policy)
lw    t2, 0(t0)         # cache miss -> load 123 from memory
lw    t3, 0(t0)         # cache hit  -> load 123 from cache
