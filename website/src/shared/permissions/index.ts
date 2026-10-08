const permissions = {
   RT: 1,
   QAT: 2,
   QATHead: 4,
   NAT: 8,
   ADMIN: 16,
   PANDA: 32,
   SUPPORTER: 64,
   PPFARMER: 128,
   DEV: 256,
   PPV3: 512,
   CCT: 1024,
   CCTHead: 2048,
   CAT: 4096,
   RTR: 8192,
   EXTERNAL_DEV: 16384,
   TOURNAMENT_ORGANIZER: 32768
};

const group_permissions = {
   ALL_STAFF:
      permissions.RT |
      permissions.QAT |
      permissions.QATHead |
      permissions.NAT |
      permissions.ADMIN |
      permissions.PANDA |
      permissions.DEV |
      permissions.PPV3 |
      permissions.CCT |
      permissions.CCTHead |
      permissions.CAT |
      permissions.RTR |
      permissions.TOURNAMENT_ORGANIZER,
   ADMIN: permissions.RT | permissions.QAT | permissions.QATHead | permissions.NAT | permissions.ADMIN,
   NAT: permissions.RT | permissions.NAT,
   QATHead: permissions.QAT | permissions.QATHead,
   QAT: permissions.QAT,
   RT: permissions.RT
};

function checkPermissionNumber(userPermissions: number, permission: number) {
   if ((userPermissions & permissions.PANDA) !== 0) return permission !== 0;
   return (userPermissions & permission) !== 0;
}

function isSupporter(_userPermissions: number) {
   // SnoreSaber currently has no paid/Patreon feature tier.
   return true;
}

function isPPFarmer(_userPermissions: number) {
   // All profile customization features are currently free for every signed-in user.
   return true;
}

export default {
   security: permissions,
   groups: group_permissions,
   checkPermissionNumber,
   isSupporter,
   isPPFarmer
};
