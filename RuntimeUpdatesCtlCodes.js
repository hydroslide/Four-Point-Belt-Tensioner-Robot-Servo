var utils = GetTensionUtils();

function SmoothItOut(servoName, val) {
  // Low-pass IIR filtering of left and right tension values
  var key = servoName + 'B4';
  if (null == root[key]) {
    root[key] = val;	// initialize
  }
  var valB4 = root[key]; // previously filtered values
  var tc = 1 + $prop('Settings.smooth');
  valB4 += (val - valB4) / tc;
  root[key] = valB4;
  return valB4;
}

function ApplyMaxMin(val, minVal, maxVal) {
  if (val > 0)
    return val * maxVal;
  else
    return val * minVal;
}

// G-forces from SimHub properties
/*
var gforce_x = $prop('ShakeITMotorsV3Plugin.Export.LateralGforce.Right')-$prop('ShakeITMotorsV3Plugin.Export.LateralGforce.Left')// acceleration
var gforce_y = $prop('ShakeITMotorsV3Plugin.Export.DecelGforce.Front') - $prop('ShakeITMotorsV3Plugin.Export.AccelGforce.Rear')// 
var gforce_z = 0;//
*/
var gForceSway = $prop('AccelerationSway');	// lateral (yaw) 
var gForceSurge = $prop('GlobalAccelerationG');	// deceleration
var gForceHeave = $prop('AccelerationHeave');

var masterGain = $prop('Settings.master_gain') / 10;

gForceSway *= ($prop('Settings.yaw_gain') / 10) * masterGain;
gForceSurge *= ($prop('Settings.decel_gain') / 10) * masterGain;
gForceHeave *= ($prop('Settings.heave_gain') / 10) * masterGain;


// if (0 > gforce_y)
//   gforce_y = 0;				// non-negative deceleration


var lsVal = 0;
var rsVal = 0;
var lwVal = 0;
var rwVal = 0;

/* // Old code
// convert speed and yaw changes to left and right tension values
// turning right should increase right harness tension (body pushed left)
var rs = Math.sqrt(gforce_y * gforce_y + gforce_x * gforce_x);
var ls = gforce_y + gforce_y - rs;
if (0 > gforce_x) {
  var t = rs;	// negative gforce_x increases left tension
  rs = ls;
  ls = t;
}

var rw = Math.sqrt(gforce_z * gforce_z + gforce_x * gforce_x);
var lw = gforce_z + gforce_z - rw;
if (0 > gforce_x) {
  var t = rw;	// negative gforce_x increases left tension
  rw = lw;
  lw = t;
}
*/

var rs = 0;
var ls = 0;
var rw = 0;
var lw = 0;

var beltForces = {
  ls: [],
  rs: [],
  lw: [],
  rw: []
}

var combineForces = function (arr) {
  //return avg(arr);
  var bestValue = 0;

  arr.forEach(function (val) {
    if (Math.abs(val) > Math.abs(bestValue))
      bestValue = val;
  });
  return bestValue;
}

var avg = function (arr) {
  if (arr.length > 0) {
    var total = 0;

    arr.forEach(function (val) {
      total += val;
    });

    return total / arr.length;
  }
  else
    return 0;
}

var positiveSurgeMultiplier = 2;//1.5

if (gForceSurge > 0)
  gForceSurge *= positiveSurgeMultiplier;

beltForces.ls.push(gForceSurge);
beltForces.rs.push(gForceSurge);
if (gForceSurge < 0) {
  beltForces.lw.push(Math.abs(gForceSurge))
  beltForces.rw.push(Math.abs(gForceSurge))
}

var shoulderSwayMultiplier = .25
if (gForceSway < 0) {
  beltForces.lw.push(gForceSway * -1);
  beltForces.ls.push(gForceSway * -1 * shoulderSwayMultiplier);
}
else {
  beltForces.rw.push(gForceSway);
  beltForces.rs.push(gForceSway * shoulderSwayMultiplier);
}

beltForces.lw.push(-1 * gForceHeave);
beltForces.rw.push(-1 * gForceHeave);

ls = combineForces(beltForces.ls);
rs = combineForces(beltForces.rs);
lw = combineForces(beltForces.lw);
rw = combineForces(beltForces.rw);

// Normalize to a value between 0 and 1
rsVal = rs / 2000;
lsVal = ls / 2000;
rwVal = rw / 2000;
lwVal = lw / 2000;

//return lsVal.toString()+","+lwVal.toString()+","+rwVal.toString()+","+rsVal.toString()




//return rsVal;

// // Assign values to belts
// // TODO: recombine the values the way you really want them. For now just put heave to waist for test
// lsVal = l;
// rsVal = r;


lsVal = SmoothItOut("ls", lsVal);
rsVal = SmoothItOut("rs", rsVal);
lwVal = SmoothItOut("lw", lwVal);
rwVal = SmoothItOut("rw", rwVal);


lsVal = ApplyMaxMin(utils.lsMin, utils.lsMax, lsVal);
rsVal = ApplyMaxMin(utils.rsMin, utils.rsMax, rsVal);
lwVal = ApplyMaxMin(utils.lwMin, utils.lwMax, lwVal);
rwVal = ApplyMaxMin(utils.rwMin, utils.rwMax, rwVal);

var concatCommand = utils.concatCommand;
var getCommandValFromAbs = utils.getCommandValFromAbs;
var applyNeutralOffset = utils.applyNeutralOffset;

var command = "";


if (!$prop('Settings.useShoulder')) {
  lsVal = 0;
  rsVal = 0;
}
if (!$prop('Settings.useWaist')) {
  lwVal = 0;
  rwVal = 0;
}
command = concatCommand(command, utils.lsCtl, getCommandValFromAbs(applyNeutralOffset(utils.lsnPos, lsVal)));
command = concatCommand(command, utils.rsCtl, getCommandValFromAbs(applyNeutralOffset(utils.rsnPos, rsVal)));
command = concatCommand(command, utils.lwCtl, getCommandValFromAbs(applyNeutralOffset(utils.lwnPos, lwVal)));
command = concatCommand(command, utils.rwCtl, getCommandValFromAbs(applyNeutralOffset(utils.rwnPos, rwVal)));

return command;
