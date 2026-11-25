.pragma library

// 24 px outline icons from the workbench prototype.
var paths = {
    grid: '<rect x="3" y="3" width="7" height="7" rx="1.5"/><rect x="14" y="3" width="7" height="7" rx="1.5"/><rect x="3" y="14" width="7" height="7" rx="1.5"/><rect x="14" y="14" width="7" height="7" rx="1.5"/>',
    pulse: '<path d="M2 12h5l3-8 4 16 3-8h5"/>',
    bell: '<path d="M18 8a6 6 0 0 0-12 0c0 7-3 8-3 10h18c0-2-3-3-3-10ZM10 21h4"/>',
    bottle: '<path d="M9.5 3h5M12 3v3m0 0c-3 0-5 1.7-5 5v8a2 2 0 0 0 2 2h6a2 2 0 0 0 2-2v-8c0-3.3-2-5-5-5ZM7 12h10"/>',
    sliders: '<path d="M4 6h7m4 0h5M4 12h2m4 0h10M4 18h10m4 0h2"/><circle cx="13" cy="6" r="2"/><circle cx="8" cy="12" r="2"/><circle cx="16" cy="18" r="2"/>',
    network: '<rect x="9" y="2.5" width="6" height="5" rx="1"/><rect x="2.5" y="16" width="6" height="5" rx="1"/><rect x="15.5" y="16" width="6" height="5" rx="1"/><path d="M12 7.5V12m-6.5 4v-4h13v4"/>',
    flow: '<rect x="2" y="3" width="7" height="6" rx="1"/><rect x="15" y="15" width="7" height="6" rx="1"/><path d="M9 6h6a3 3 0 0 1 3 3v6M5 9v7a2 2 0 0 0 2 2h8"/>',
    up: '<path d="M12 16V3m-5 5 5-5 5 5M4 14v5a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2v-5"/>',
    log: '<rect x="5" y="3" width="14" height="18" rx="2"/><path d="M9 8h6m-6 4h6m-6 4h4"/>',
    terminal: '<rect x="2.5" y="4" width="19" height="16" rx="2"/><path d="m6 9 3 3-3 3m6 0h5"/>',
    settings: '<path d="m9 3 .6-1h4.8l.6 2 2 .9 2-.4 2.4 4.1-1.4 1.6v2.4l1.4 1.6-2.4 4.1-2-.4-2 .9-.6 2H9.6l-.6-2-2-.9-2 .4-2.4-4.1L4 12.6v-2.4L2.6 8.6 5 4.5l2 .4 2-.9Z"/><circle cx="12" cy="11.5" r="3.2"/>',
    help: '<circle cx="12" cy="12" r="9"/><path d="M9.4 9a2.8 2.8 0 1 1 4.1 2.5c-1 .5-1.5 1-1.5 2.5m0 2.8v.2"/>',
    folder: '<path d="M3 6a2 2 0 0 1 2-2h4l2 2h8a2 2 0 0 1 2 2v10a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2Z"/>',
    chevron: '<path d="m9 5 7 7-7 7"/>',
    expand: '<path d="m6 9 6 6 6-6"/>',
    search: '<circle cx="10.5" cy="10.5" r="6.5"/><path d="m16 16 5 5"/>',
    menu: '<path d="M3 6h18M3 12h18M3 18h18"/>',
    review: '<path d="M13 4H5a2 2 0 0 0-2 2v13l4-3h7a2 2 0 0 0 2-2v-1m-7 0 1-4 7-7 3 3-7 7-4 1Z"/>',
    user: '<circle cx="12" cy="8" r="4"/><path d="M4 21v-2a8 8 0 0 1 16 0v2"/>',
    shield: '<path d="M12 2 3.5 5.5v6c0 5 4 8.5 8.5 11 4.5-2.5 8.5-6 8.5-11v-6Z"/><path d="m8 12 3 3 5-6"/>',
    layer: '<path d="m2 8 10-5 10 5-10 5Zm0 4 10 5 10-5M2 16l10 5 10-5"/>',
    users: '<circle cx="9" cy="8" r="3"/><path d="M3 20v-2a6 6 0 0112 0v2M15 5a3 3 0 010 6m2 3a5 5 0 014 5"/>',
    monitor: '<path d="M3 4h18v13H3zM8 21h8m-4-4v4"/>',
    key: '<circle cx="8" cy="9" r="5"/><path d="m12 12 8 8 2-2-2-2 1-1-3-3"/>'
}
