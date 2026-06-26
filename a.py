# Made by Dark Koopatrol
# calcRNG and inverseRNG taken from Blizzaria
# upd, previous, updn taken from idk


startRNG = 0x6D28FD77
advances = 2829

multi = 0x5D588B65
addi = 1
modulo = 2**32
initial = startRNG
RNG_MULTIPLIER_INV = 1786162797
MAX_RNG = (2**32) - 1
x = 1
y = 0

# next rng
def upd(rng):
    return (rng * multi + addi) % 0x100000000

# previous rng
def previous(rng):
    return (rng - 1) * RNG_MULTIPLIER_INV % 0x100000000

def extendedGCD(integer, mod):
    global x, y
    if (mod == 0):
        x = 1
        y = 0
        return integer
    nextX, nextY = 0, 0
    GCD = extendedGCD(mod, integer % mod)
    nextX, nextY = x, y
    x = nextY
    y = nextX - nextY * (integer // mod)
    return GCD

def binaryExpoMod(base, power, mod):
    if (power == 0):
        return 1
    firstFactor = binaryExpoMod(base, power // 2, mod)
    secondFactor = 1
    if (power % 2 == 1):
        secondFactor = base
    return (firstFactor * firstFactor * secondFactor) % mod

def modInverse(integer, mod):
    if (extendedGCD(integer, mod) != 1):
        return
    return (x % mod + mod) % mod

def calcRNG(index): # RNG(n, k) = (k * a^n) + (b * (a^n - 1)/(a - 1)), k = initial seed
    verticalShift = initial * binaryExpoMod(multi, index, modulo)
    geometricSeries = addi * ((binaryExpoMod(multi, index, 4*modulo) - 1) // 4 * modInverse((multi - 1) // 4, modulo))
    return (verticalShift + geometricSeries) % modulo

def pAdicValuation(prime, integer):
    if (integer == 0):
        return 0x1000000000
    
    power = 0
    integerCopy = integer
    
    while (integerCopy % prime == 0):
        integerCopy = integerCopy // prime
        power = power + 1
    return power

def inverseRNG(seed):
    guess = 0
    for power in range(0, 32):
        output = 2**power
        if (pAdicValuation(2, calcRNG(guess + output) - seed) > pAdicValuation(2, calcRNG(guess) - seed)):
            guess = guess + output
    return guess

# rng after n advances
def updn(rng, n):
    return calcRNG(inverseRNG(rng) + n)

def loopsCalc(loopValues): # loopValues formula (_rand): (MAX_RNG % (maxVal + 1)) + 1
    global rng
    rng = upd(rng)
    while (rng > MAX_RNG - loopValues):
        rng = upd(rng)

def rngStylish():
    for x in range (39):
        loopsCalc(32768)
        for y in range(2):
            loopsCalc(32768)
    for x in range (11):
        for y in range (4):
            loopsCalc(32768)
    for x in range (200):
        loopsCalc(46)
    loopsCalc(7843)
    for x in range (16):
        for y in range (3):
            loopsCalc(120)
        for y in range (2):
            loopsCalc(32768)
        for y in range(3):
            loopsCalc(4)

def rngBounce():
    for x in range (11):
        loopsCalc(120)
    loopsCalc(96)
    loopsCalc(7843)

def rngFlip():
    loopsCalc(7843)

def rngDamage():
    loopsCalc(7211)

def endRNG():
    # 0 is flip, 1 is damage, 2 is bounce, 3 is stylish
    global rng, endInputs, possible, calledStylish
    targetRNG = inverseRNG(rng)
    rng = previous(rng)
    if rng > MAX_RNG - 4:
        possible = 0
    elif rng > MAX_RNG - 7211:
        #last call is stylish
        oldRNG = rng
        rng = updn(rng, -489)
        if inverseRNG(rng) > targetRNG:
            possible = 0
            return
        oldRNG = rng
        rngStylish()
        while inverseRNG(rng) > targetRNG:
            rng = updn(rng, -491)
            if inverseRNG(rng) > targetRNG:
                possible = 0
                return
            oldRNG = rng
            rngStylish()
        if inverseRNG(rng) < targetRNG:
            possible = 0
            return
        
        targetRNG = inverseRNG(oldRNG)
        rng = updn(oldRNG, -13)
        if inverseRNG(rng) > targetRNG:
            possible = 0
            return
        oldRNG = rng
        rngBounce()
        while inverseRNG(rng) > targetRNG:
            rng = updn(rng, -14)
            if inverseRNG(rng) > targetRNG:
                possible = 0
                return
            oldRNG = rng
            rngBounce()
        if inverseRNG(rng) < targetRNG:
            possible = 0
            return
        
        endInputs.insert(0, 3)
        endInputs.insert(0, 2)
        calledStylish = 1
        rng = oldRNG
        endRNG()
    elif rng > MAX_RNG - 7843:
        #last call is damage
        endInputs.insert(0, 1)
        endRNG(rng)
    
    targetRNG = inverseRNG(upd(rng))
    return targetRNG


possible = 1
oldTargetRNG = advances
rng = calcRNG(advances)
endInputs = []

calledStylish = 0
forceEven = 0
endRNG()

if calledStylish == 1:
    targetRNG = inverseRNG(rng)
    rng = previous(rng)
    if inverseRNG(rng) > targetRNG:
        possible = 0
    else:
        oldRNG = rng
        rngFlip()
        while inverseRNG(rng) > targetRNG:
            rng = updn(rng, -2)
            if inverseRNG(rng) > targetRNG:
                possible = 0
                break
            oldRNG = rng
            rngFlip()
        if inverseRNG(rng) < targetRNG:
            possible = 0
        endInputs.insert(0, 0)
        rng = previous(oldRNG)
        if inverseRNG(rng) > targetRNG:
            possible = 0
        targetRNG = inverseRNG(rng)
        forceEven = 1 # You can only stylish in 2D, so there must be an even amount of flips

targetRNG = endRNG() # Calling endRNG again since the previous call cleared up a slot for a flip which is added later

if possible == 0:
    print("Not possible to reach desired RNG value")
else:
    advances = advances - (oldTargetRNG - targetRNG)
    rng = startRNG
    stylishes = advances // (490 + 13)
    if stylishes > 0:
        for x in range (stylishes-1):
            rngBounce()
            rngStylish()
        oldRNG = rng
        rngBounce()
        rngStylish()
        if inverseRNG(rng) > targetRNG:
            stylishes = stylishes - 1
            rng = oldRNG
        advances = targetRNG - inverseRNG(rng)

    bounces = advances // 13
    if bounces > 0:
        for x in range (bounces-1):
            rngBounce()
        oldRNG = rng
        rngBounce()
        if inverseRNG(rng) > targetRNG:
            bounces = bounces - 1
            rng = oldRNG
        advances = targetRNG - inverseRNG(rng)

    bounces = bounces + stylishes
    flips = 1 # This is the flip to get behind the wall that was calculated earlier
    while(inverseRNG(rng) < targetRNG):
        rngFlip()
        flips = flips + 1
    
    damage = 0
    if (forceEven == 1) and (flips % 2 == 0): # Even is odd because one of the flips is stored in endInputs
        flips = flips - 1
        damage = damage + 1
    elif (flips % 2 == 0):
        flips = flips - 1
        endInputs.insert(0, 0)
    
    print(stylishes, "stylishes")
    print(bounces, "bounces")
    print(flips, "flips")
    
    if damage > 0:
        print(damage, "damages")
    
    print("Go past wall")

    for x in range (0, len(endInputs)):
        match endInputs[x]:
            case 0:
                print("1 flip")
            case 1:
                print("1 damage")
            case 2:
                print("1 bounce")
            case 3:
                print("1 stylish")