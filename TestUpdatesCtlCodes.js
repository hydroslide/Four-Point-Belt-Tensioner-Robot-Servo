var utils = GetTensionUtils();

if ($prop('Settings.TestNeutralTensions')) {
  var command = "";
  command = utils.concatCommand(command, utils.lsCtl, utils.getCommandValFromAbs(utils.lsnPos));
  command = utils.concatCommand(command, utils.rsCtl, utils.getCommandValFromAbs(utils.rsnPos));
  command = utils.concatCommand(command, utils.lwCtl, utils.getCommandValFromAbs(utils.lwnPos));
  command = utils.concatCommand(command, utils.rwCtl, utils.getCommandValFromAbs(utils.rwnPos));
  return command;
} else if ($prop('Settings.TestMaxTensions')) {
  var command = "";
  command = utils.concatCommand(command, utils.lsCtl, utils.getCommandValFromAbs(utils.getMaxTensionPos(utils.lsnPos, utils.lsMax)));
  command = utils.concatCommand(command, utils.rsCtl, utils.getCommandValFromAbs(utils.getMaxTensionPos(utils.rsnPos, utils.rsMax)));
  command = utils.concatCommand(command, utils.lwCtl, utils.getCommandValFromAbs(utils.getMaxTensionPos(utils.lwnPos, utils.lwMax)));
  command = utils.concatCommand(command, utils.rwCtl, utils.getCommandValFromAbs(utils.getMaxTensionPos(utils.rwnPos, utils.rwMax)));
  return command;
}else if ($prop('Settings.TestMinTensions')) {
  var command = "";
  command = utils.concatCommand(command, utils.lsCtl, utils.getCommandValFromAbs(utils.getMinTensionPos(utils.lsnPos, utils.lsMin)));
  command = utils.concatCommand(command, utils.rsCtl, utils.getCommandValFromAbs(utils.getMinTensionPos(utils.rsnPos, utils.rsMin)));
  command = utils.concatCommand(command, utils.lwCtl, utils.getCommandValFromAbs(utils.getMinTensionPos(utils.lwnPos, utils.lwMin)));
  command = utils.concatCommand(command, utils.rwCtl, utils.getCommandValFromAbs(utils.getMinTensionPos(utils.rwnPos, utils.rwMin)));
  return command;
}
