
// run(1.07142394, 64, true); // to 256 in 64 steps

// run(1.07484, 100, true); // to 4096 in 100 steps

run(1.1279, 64, true); // to 4096 in 64 steps

// iterate(1.1279, 64, 4096, 0.000001);



function iterate(mult, steps, max, by) {

    for (var i = 1; i <= 500; i++) {

        var result = run(mult, steps);

        var diff = max - result;

        console.log('%d multiplier, maxed at %d, off by %d', mult, result, diff);

        if (diff > 0) {
            mult += by;
        } else {
            break;
        }

    }
}


function run(mult, steps, print) {

    var out = 1;
    var int = 0;
    var prev = 0;

    for (var i = 1; i <= steps; i++) {

        out = out * mult;
        int = parseInt(out, 10);
        out = (int == prev) ? int + 1 : int;
        int = parseInt(out, 10);
        prev = int;

        var voltage = (5 / 4096 * int).toPrecision(4);

        if (print) { console.log('Step %d: val of %d for %d volts', i, int, voltage); }

    }

    return int;

}