



let systemData = {
    cities: [],
    routes: [],
    parcels: [],
    riders: [],
    nextTrackingId: 1000,
    nextRiderId: 1
};


document.addEventListener('DOMContentLoaded', () => {
    initializeApp();
});

async function initializeApp() {
    
    await loadDataFromJSON();
    
    
    setupEventListeners();
    
    
    const isLoggedIn = sessionStorage.getItem('isLoggedIn');
    if (isLoggedIn === 'true') {
        showDashboard();
    }
}


async function loadDataFromJSON() {
    try {
        console.log('Loading data from API...');
        const [cities, routes, parcels, riders] = await Promise.all([
            fetch('/api/cities').then(r => r.json()),
            fetch('/api/routes').then(r => r.json()),
            fetch('/api/parcels').then(r => r.json()),
            fetch('/api/riders').then(r => r.json())
        ]);
        
        systemData.cities = cities.cities || [];
        systemData.routes = routes.routes || [];
        systemData.parcels = parcels.parcels || [];
        systemData.riders = riders.riders || [];
        systemData.nextTrackingId = parcels.nextTrackingId || 1000;
        systemData.nextRiderId = riders.nextRiderId || 1;
        
        console.log('Data loaded successfully:', {
            cities: systemData.cities.length,
            routes: systemData.routes.length,
            parcels: systemData.parcels.length,
            riders: systemData.riders.length
        });
        
    } catch (error) {
        console.error('Error loading data:', error);
        showToast('Error loading data. Make sure the server is running.', 'error');
    }
}


async function saveDataToJSON() {
    try {
        
        console.log('Data saved:', systemData);
        
        
        localStorage.setItem('systemData', JSON.stringify(systemData));
        
        return true;
    } catch (error) {
        console.error('Error saving data:', error);
        return false;
    }
}


function loadFromLocalStorage() {
    const stored = localStorage.getItem('systemData');
    if (stored) {
        systemData = JSON.parse(stored);
    }
}


function setupEventListeners() {
    
    document.getElementById('loginForm').addEventListener('submit', handleLogin);
    document.getElementById('logoutBtn').addEventListener('click', handleLogout);
    
    
    document.querySelectorAll('.menu-item').forEach(item => {
        item.addEventListener('click', handleMenuClick);
    });
    
    
    document.getElementById('addParcelBtn').addEventListener('click', showAddParcelModal);
    document.getElementById('parcelSearch').addEventListener('input', filterParcels);
    document.getElementById('statusFilter').addEventListener('change', filterParcels);
    
    
    document.getElementById('trackParcelBtn').addEventListener('click', trackParcel);
    
    
    document.getElementById('calculateRouteBtn').addEventListener('click', calculateRoute);
    document.getElementById('blockRouteBtn').addEventListener('click', showBlockRouteModal);
    document.getElementById('viewBlockedRoutesBtn').addEventListener('click', viewBlockedRoutes);
    
    
    document.getElementById('addRiderBtn').addEventListener('click', showAddRiderModal);
    
    
    document.getElementById('addCityBtn').addEventListener('click', showAddCityModal);
    document.getElementById('addRouteBtn').addEventListener('click', showAddRouteModal);
    
    
    document.getElementById('pickupParcelBtn').addEventListener('click', pickupParcel);
    document.getElementById('moveToTransitBtn').addEventListener('click', moveToTransit);
    document.getElementById('deliverParcelBtn').addEventListener('click', deliverParcel);
    document.getElementById('assignRiderBtn').addEventListener('click', showAssignRiderModal);
    
    
    document.getElementById('viewParcelStatsBtn').addEventListener('click', viewParcelStats);
    document.getElementById('viewRiderStatsBtn').addEventListener('click', viewRiderStats);
    document.getElementById('viewMissingParcelsBtn').addEventListener('click', viewMissingParcels);
}


function handleLogin(e) {
    e.preventDefault();
    const username = document.getElementById('username').value;
    const password = document.getElementById('password').value;
    
    
    if (username && password) {
        sessionStorage.setItem('isLoggedIn', 'true');
        sessionStorage.setItem('username', username);
        showDashboard();
    } else {
        showLoginError('Please enter username and password');
    }
}

function handleLogout() {
    sessionStorage.removeItem('isLoggedIn');
    sessionStorage.removeItem('username');
    showLoginScreen();
}

function showLoginError(message) {
    const errorDiv = document.getElementById('loginError');
    errorDiv.textContent = message;
    errorDiv.classList.add('show');
    setTimeout(() => {
        errorDiv.classList.remove('show');
    }, 3000);
}

function showLoginScreen() {
    document.getElementById('loginScreen').classList.add('active');
    document.getElementById('dashboardScreen').classList.remove('active');
}

async function showDashboard() {
    document.getElementById('loginScreen').classList.remove('active');
    document.getElementById('dashboardScreen').classList.add('active');
    loadFromLocalStorage();
    
    
    await loadDataFromJSON();
    
    updateDashboard();
}


function handleMenuClick(e) {
    const menuItem = e.currentTarget;
    const section = menuItem.dataset.section;
    
    
    document.querySelectorAll('.menu-item').forEach(item => {
        item.classList.remove('active');
    });
    menuItem.classList.add('active');
    
    
    document.querySelectorAll('.content-section').forEach(sec => {
        sec.classList.remove('active');
    });
    document.getElementById(`${section}Section`).classList.add('active');
    
    
    loadSectionData(section);
}


async function loadSectionData(section) {
    
    if (systemData.cities.length === 0) {
        await loadDataFromJSON();
    }
    
    switch(section) {
        case 'overview':
            updateDashboard();
            break;
        case 'parcels':
            loadParcelsTable();
            break;
        case 'tracking':
            
            break;
        case 'routing':
            loadRoutingData();
            break;
        case 'riders':
            loadRidersTable();
            break;
        case 'cities':
            loadCitiesAndRoutes();
            break;
        case 'operations':
            
            break;
        case 'reports':
            
            break;
    }
}


function updateDashboard() {
    
    const totalParcels = systemData.parcels.length;
    const deliveredParcels = systemData.parcels.filter(p => p.status === 'Delivered').length;
    const inTransitParcels = systemData.parcels.filter(p => p.status === 'InTransit').length;
    const totalRiders = systemData.riders.length;
    
    document.getElementById('totalParcels').textContent = totalParcels;
    document.getElementById('deliveredParcels').textContent = deliveredParcels;
    document.getElementById('inTransitParcels').textContent = inTransitParcels;
    document.getElementById('totalRiders').textContent = totalRiders;
    
    
    const pickupQueue = systemData.parcels.filter(p => p.status === 'Pending').length;
    const warehouseQueue = systemData.parcels.filter(p => p.status === 'InWarehouse').length;
    const transitQueue = systemData.parcels.filter(p => p.status === 'InTransit').length;
    
    document.getElementById('pickupQueue').textContent = pickupQueue;
    document.getElementById('warehouseQueue').textContent = warehouseQueue;
    document.getElementById('transitQueue').textContent = transitQueue;
    
    
    updateRecentActivity();
}

function updateRecentActivity() {
    const activityContainer = document.getElementById('recentActivity');
    if (systemData.parcels.length === 0) {
        activityContainer.innerHTML = '<p class="text-muted">No recent activity</p>';
        return;
    }
    
    
    const recentParcels = systemData.parcels.slice(-5).reverse();
    const html = recentParcels.map(parcel => `
        <div class="activity-item">
            <strong>Parcel #${parcel.trackingId}</strong><br>
            <small>${parcel.sender} → ${parcel.receiver}</small><br>
            <small class="text-muted">Status: ${getStatusBadge(parcel.status)}</small>
        </div>
    `).join('');
    
    activityContainer.innerHTML = html;
}


function loadParcelsTable() {
    filterParcels(); 
}

function filterParcels() {
    const searchTerm = document.getElementById('parcelSearch').value.toLowerCase();
    const statusFilter = document.getElementById('statusFilter').value;
    
    let filteredParcels = systemData.parcels.filter(parcel => {
        const matchesSearch = searchTerm === '' || 
            parcel.trackingId.toString().includes(searchTerm) ||
            parcel.sender.toLowerCase().includes(searchTerm) ||
            parcel.receiver.toLowerCase().includes(searchTerm);
        
        const matchesStatus = statusFilter === '' || parcel.status === statusFilter;
        
        return matchesSearch && matchesStatus;
    });
    
    const tbody = document.getElementById('parcelsTableBody');
    if (filteredParcels.length === 0) {
        tbody.innerHTML = '<tr><td colspan="8" class="text-center text-muted">No parcels found</td></tr>';
        return;
    }
    
    const html = filteredParcels.map(parcel => {
        const sourceCity = getCityName(parcel.sourceCityId);
        const destCity = getCityName(parcel.destCityId);
        
        return `
            <tr>
                <td><strong>#${parcel.trackingId}</strong></td>
                <td>${parcel.sender}</td>
                <td>${parcel.receiver}</td>
                <td>${getPriorityBadge(parcel.priority)}</td>
                <td>${parcel.weight} kg</td>
                <td>${getStatusBadge(parcel.status)}</td>
                <td>${sourceCity} → ${destCity}</td>
                <td class="action-btns">
                    <button class="btn-icon" onclick="viewParcelDetails(${parcel.trackingId})" title="View Details">
                        <i class="fas fa-eye"></i>
                    </button>
                    <button class="btn-icon" onclick="editParcel(${parcel.trackingId})" title="Edit">
                        <i class="fas fa-edit"></i>
                    </button>
                    <button class="btn-icon" onclick="deleteParcel(${parcel.trackingId})" title="Delete">
                        <i class="fas fa-trash"></i>
                    </button>
                </td>
            </tr>
        `;
    }).join('');
    
    tbody.innerHTML = html;
}

function showAddParcelModal() {
    const modal = createModal('Add New Parcel', `
        <form id="addParcelForm">
            <div class="form-group">
                <label>Sender Name:</label>
                <input type="text" id="senderName" class="form-control" required>
            </div>
            <div class="form-group">
                <label>Receiver Name:</label>
                <input type="text" id="receiverName" class="form-control" required>
            </div>
            <div class="form-group">
                <label>Priority:</label>
                <select id="parcelPriority" class="form-control" required>
                    <option value="1">Overnight</option>
                    <option value="2">Two-Day</option>
                    <option value="3">Normal</option>
                </select>
            </div>
            <div class="form-group">
                <label>Weight (kg):</label>
                <input type="number" id="parcelWeight" class="form-control" min="0.1" step="0.1" required>
            </div>
            <div class="form-group">
                <label>Source City:</label>
                <select id="parcelSourceCity" class="form-control" required>
                    ${getCitiesOptions()}
                </select>
            </div>
            <div class="form-group">
                <label>Destination City:</label>
                <select id="parcelDestCity" class="form-control" required>
                    ${getCitiesOptions()}
                </select>
            </div>
            <div class="form-group">
                <label>
                    <input type="checkbox" id="parcelFragile">
                    Fragile Item
                </label>
            </div>
        </form>
    `, [
        { text: 'Cancel', class: 'btn-secondary', onClick: closeModal },
        { text: 'Add Parcel', class: 'btn-primary', onClick: addParcel }
    ]);
    
    showModal(modal);
}

function addParcel() {
    const sender = document.getElementById('senderName').value;
    const receiver = document.getElementById('receiverName').value;
    const priority = parseInt(document.getElementById('parcelPriority').value);
    const weight = parseFloat(document.getElementById('parcelWeight').value);
    const sourceCityId = parseInt(document.getElementById('parcelSourceCity').value);
    const destCityId = parseInt(document.getElementById('parcelDestCity').value);
    const isFragile = document.getElementById('parcelFragile').checked;
    
    if (!sender || !receiver || !weight || !sourceCityId || !destCityId) {
        showToast('Please fill all required fields', 'error');
        return;
    }
    
    if (sourceCityId === destCityId) {
        showToast('Source and destination cities must be different', 'error');
        return;
    }
    
    const newParcel = {
        trackingId: systemData.nextTrackingId++,
        sender,
        receiver,
        priority,
        weight,
        status: 'Pending',
        sourceCityId,
        destCityId,
        currentCityId: sourceCityId,
        deliveryAttempts: 0,
        isFragile,
        history: [`${new Date().toISOString()} - Parcel Created`],
        currentRoute: []
    };
    
    systemData.parcels.push(newParcel);
    saveDataToJSON();
    
    closeModal();
    loadParcelsTable();
    updateDashboard();
    showToast('Parcel added successfully', 'success');
}

function deleteParcel(trackingId) {
    if (!confirm('Are you sure you want to delete this parcel?')) {
        return;
    }
    
    const index = systemData.parcels.findIndex(p => p.trackingId === trackingId);
    if (index !== -1) {
        systemData.parcels.splice(index, 1);
        saveDataToJSON();
        loadParcelsTable();
        updateDashboard();
        showToast('Parcel deleted successfully', 'success');
    }
}


function trackParcel() {
    const trackingId = parseInt(document.getElementById('trackingIdInput').value);
    const parcel = systemData.parcels.find(p => p.trackingId === trackingId);
    
    const resultDiv = document.getElementById('trackingResult');
    
    if (!parcel) {
        resultDiv.innerHTML = `
            <div class="text-center text-muted">
                <i class="fas fa-search" style="font-size: 3rem; margin-bottom: 1rem;"></i>
                <p>No parcel found with tracking ID: ${trackingId}</p>
            </div>
        `;
        return;
    }
    
    const sourceCity = getCityName(parcel.sourceCityId);
    const destCity = getCityName(parcel.destCityId);
    const currentCity = getCityName(parcel.currentCityId);
    
    const historyHtml = parcel.history.map((item, index) => `
        <div class="history-item ${index === parcel.history.length - 1 ? 'active' : ''}">
            ${item}
        </div>
    `).join('');
    
    resultDiv.innerHTML = `
        <div class="tracking-card">
            <h3>Tracking ID: #${parcel.trackingId}</h3>
            <div class="tracking-info">
                <div class="tracking-info-item">
                    <label>Status</label>
                    <span>${getStatusBadge(parcel.status)}</span>
                </div>
                <div class="tracking-info-item">
                    <label>Priority</label>
                    <span>${getPriorityBadge(parcel.priority)}</span>
                </div>
                <div class="tracking-info-item">
                    <label>Weight</label>
                    <span>${parcel.weight} kg</span>
                </div>
                <div class="tracking-info-item">
                    <label>Sender</label>
                    <span>${parcel.sender}</span>
                </div>
                <div class="tracking-info-item">
                    <label>Receiver</label>
                    <span>${parcel.receiver}</span>
                </div>
                <div class="tracking-info-item">
                    <label>From</label>
                    <span>${sourceCity}</span>
                </div>
                <div class="tracking-info-item">
                    <label>To</label>
                    <span>${destCity}</span>
                </div>
                <div class="tracking-info-item">
                    <label>Current Location</label>
                    <span>${currentCity}</span>
                </div>
            </div>
        </div>
        
        <div class="tracking-history">
            <h3>Tracking History</h3>
            <div class="history-timeline">
                ${historyHtml}
            </div>
        </div>
    `;
}


function loadRoutingData() {
    const sourceCitySelect = document.getElementById('routeSourceCity');
    const destCitySelect = document.getElementById('routeDestCity');
    
    sourceCitySelect.innerHTML = getCitiesOptions();
    destCitySelect.innerHTML = getCitiesOptions();
}

function calculateRoute() {
    const sourceCityId = parseInt(document.getElementById('routeSourceCity').value);
    const destCityId = parseInt(document.getElementById('routeDestCity').value);
    
    if (!sourceCityId || !destCityId) {
        showToast('Please select both cities', 'error');
        return;
    }
    
    if (sourceCityId === destCityId) {
        showToast('Source and destination must be different', 'error');
        return;
    }
    
    
    const result = dijkstraShortestPath(sourceCityId, destCityId);
    
    const resultDiv = document.getElementById('routeResult');
    
    if (!result || result.distance === Infinity) {
        resultDiv.innerHTML = `
            <div class="text-center text-muted">
                <i class="fas fa-exclamation-circle" style="font-size: 3rem; margin-bottom: 1rem; color: var(--warning-color);"></i>
                <p>No route found between these cities.</p>
            </div>
        `;
        return;
    }
    
    const pathHtml = result.path.map((cityId, index) => {
        const cityName = getCityName(cityId);
        const arrow = index < result.path.length - 1 ? '<i class="fas fa-arrow-right route-arrow"></i>' : '';
        return `<span class="route-city">${cityName}</span>${arrow}`;
    }).join('');
    
    resultDiv.innerHTML = `
        <h3>Optimal Route</h3>
        <p><strong>Total Distance:</strong> ${result.distance} km</p>
        <p><strong>Number of Cities:</strong> ${result.path.length}</p>
        <div class="route-path">
            ${pathHtml}
        </div>
    `;
}


function dijkstraShortestPath(sourceId, destId) {
    const distances = {};
    const previous = {};
    const visited = new Set();
    const queue = [];
    
    
    systemData.cities.forEach(city => {
        distances[city.id] = Infinity;
        previous[city.id] = null;
    });
    
    distances[sourceId] = 0;
    queue.push({ id: sourceId, distance: 0 });
    
    while (queue.length > 0) {
        
        queue.sort((a, b) => a.distance - b.distance);
        const current = queue.shift();
        
        if (visited.has(current.id)) continue;
        visited.add(current.id);
        
        if (current.id === destId) break;
        
        
        const neighbors = systemData.routes.filter(r => r.sourceId === current.id || r.destId === current.id);
        
        neighbors.forEach(route => {
            const neighborId = route.sourceId === current.id ? route.destId : route.sourceId;
            
            if (visited.has(neighborId)) return;
            
            const newDistance = distances[current.id] + route.distance;
            
            if (newDistance < distances[neighborId]) {
                distances[neighborId] = newDistance;
                previous[neighborId] = current.id;
                queue.push({ id: neighborId, distance: newDistance });
            }
        });
    }
    
    
    const path = [];
    let currentId = destId;
    
    while (currentId !== null) {
        path.unshift(currentId);
        currentId = previous[currentId];
    }
    
    if (path[0] !== sourceId) {
        return null; 
    }
    
    return {
        distance: distances[destId],
        path
    };
}

function showBlockRouteModal() {
    const modal = createModal('Block Route', `
        <form id="blockRouteForm">
            <div class="form-group">
                <label>Source City:</label>
                <select id="blockSourceCity" class="form-control" required>
                    ${getCitiesOptions()}
                </select>
            </div>
            <div class="form-group">
                <label>Destination City:</label>
                <select id="blockDestCity" class="form-control" required>
                    ${getCitiesOptions()}
                </select>
            </div>
        </form>
    `, [
        { text: 'Cancel', class: 'btn-secondary', onClick: closeModal },
        { text: 'Block Route', class: 'btn-warning', onClick: blockRoute }
    ]);
    
    showModal(modal);
}

function blockRoute() {
    const sourceCityId = parseInt(document.getElementById('blockSourceCity').value);
    const destCityId = parseInt(document.getElementById('blockDestCity').value);
    
    if (!sourceCityId || !destCityId || sourceCityId === destCityId) {
        showToast('Please select valid cities', 'error');
        return;
    }
    
    
    const route = systemData.routes.find(r => 
        (r.sourceId === sourceCityId && r.destId === destCityId) ||
        (r.sourceId === destCityId && r.destId === sourceCityId)
    );
    
    if (route) {
        route.blocked = true;
        saveDataToJSON();
        closeModal();
        showToast('Route blocked successfully', 'success');
    } else {
        showToast('Route not found', 'error');
    }
}

function viewBlockedRoutes() {
    const blockedRoutes = systemData.routes.filter(r => r.blocked);
    
    if (blockedRoutes.length === 0) {
        showToast('No blocked routes', 'info');
        return;
    }
    
    const html = blockedRoutes.map(route => {
        const sourceCity = getCityName(route.sourceId);
        const destCity = getCityName(route.destId);
        return `<li>${sourceCity} ↔ ${destCity} (${route.distance} km)</li>`;
    }).join('');
    
    const modal = createModal('Blocked Routes', `
        <ul style="list-style: none; padding: 0;">
            ${html}
        </ul>
    `, [
        { text: 'Close', class: 'btn-secondary', onClick: closeModal }
    ]);
    
    showModal(modal);
}


function loadRidersTable() {
    const tbody = document.getElementById('ridersTableBody');
    
    if (systemData.riders.length === 0) {
        tbody.innerHTML = '<tr><td colspan="7" class="text-center text-muted">No riders found</td></tr>';
        return;
    }
    
    const html = systemData.riders.map(rider => {
        const location = getCityName(rider.currentCityId);
        const assignedCount = rider.assignedParcels ? rider.assignedParcels.length : 0;
        
        return `
            <tr>
                <td><strong>#${rider.riderId}</strong></td>
                <td>${rider.name}</td>
                <td>${rider.capacity} kg</td>
                <td>${rider.currentLoad || 0} kg</td>
                <td>${location}</td>
                <td>${assignedCount}</td>
                <td class="action-btns">
                    <button class="btn-icon" onclick="viewRiderDetails(${rider.riderId})" title="View Details">
                        <i class="fas fa-eye"></i>
                    </button>
                    <button class="btn-icon" onclick="deleteRider(${rider.riderId})" title="Delete">
                        <i class="fas fa-trash"></i>
                    </button>
                </td>
            </tr>
        `;
    }).join('');
    
    tbody.innerHTML = html;
}

function showAddRiderModal() {
    const modal = createModal('Add New Rider', `
        <form id="addRiderForm">
            <div class="form-group">
                <label>Rider Name:</label>
                <input type="text" id="riderName" class="form-control" required>
            </div>
            <div class="form-group">
                <label>Capacity (kg):</label>
                <input type="number" id="riderCapacity" class="form-control" min="1" required>
            </div>
            <div class="form-group">
                <label>Current Location:</label>
                <select id="riderLocation" class="form-control" required>
                    ${getCitiesOptions()}
                </select>
            </div>
        </form>
    `, [
        { text: 'Cancel', class: 'btn-secondary', onClick: closeModal },
        { text: 'Add Rider', class: 'btn-primary', onClick: addRider }
    ]);
    
    showModal(modal);
}

function addRider() {
    const name = document.getElementById('riderName').value;
    const capacity = parseInt(document.getElementById('riderCapacity').value);
    const currentCityId = parseInt(document.getElementById('riderLocation').value);
    
    if (!name || !capacity || !currentCityId) {
        showToast('Please fill all fields', 'error');
        return;
    }
    
    const newRider = {
        riderId: systemData.nextRiderId++,
        name,
        capacity,
        currentCityId,
        currentLoad: 0,
        assignedParcels: []
    };
    
    systemData.riders.push(newRider);
    saveDataToJSON();
    
    closeModal();
    loadRidersTable();
    updateDashboard();
    showToast('Rider added successfully', 'success');
}

function deleteRider(riderId) {
    if (!confirm('Are you sure you want to delete this rider?')) {
        return;
    }
    
    const index = systemData.riders.findIndex(r => r.riderId === riderId);
    if (index !== -1) {
        systemData.riders.splice(index, 1);
        saveDataToJSON();
        loadRidersTable();
        updateDashboard();
        showToast('Rider deleted successfully', 'success');
    }
}


function loadCitiesAndRoutes() {
    loadCitiesList();
    loadRoutesList();
}

function loadCitiesList() {
    const container = document.getElementById('citiesList');
    
    if (systemData.cities.length === 0) {
        container.innerHTML = '<p class="text-muted">No cities found</p>';
        return;
    }
    
    const html = systemData.cities.map(city => `
        <div class="list-item">
            <span><strong>${city.name}</strong> (ID: ${city.id})</span>
        </div>
    `).join('');
    
    container.innerHTML = html;
}

function loadRoutesList() {
    const container = document.getElementById('routesList');
    
    if (systemData.routes.length === 0) {
        container.innerHTML = '<p class="text-muted">No routes found</p>';
        return;
    }
    
    const html = systemData.routes.map(route => {
        const sourceCity = getCityName(route.sourceId);
        const destCity = getCityName(route.destId);
        const blockedTag = route.blocked ? ' <span class="badge badge-returned">BLOCKED</span>' : '';
        
        return `
            <div class="list-item">
                <span>${sourceCity} ↔ ${destCity}${blockedTag}</span>
                <span>${route.distance} km</span>
            </div>
        `;
    }).join('');
    
    container.innerHTML = html;
}

function showAddCityModal() {
    const modal = createModal('Add New City', `
        <form id="addCityForm">
            <div class="form-group">
                <label>City Name:</label>
                <input type="text" id="cityName" class="form-control" required>
            </div>
        </form>
    `, [
        { text: 'Cancel', class: 'btn-secondary', onClick: closeModal },
        { text: 'Add City', class: 'btn-primary', onClick: addCity }
    ]);
    
    showModal(modal);
}

function addCity() {
    const name = document.getElementById('cityName').value;
    
    if (!name) {
        showToast('Please enter city name', 'error');
        return;
    }
    
    const maxId = Math.max(...systemData.cities.map(c => c.id), 0);
    const newCity = {
        id: maxId + 1,
        name
    };
    
    systemData.cities.push(newCity);
    saveDataToJSON();
    
    closeModal();
    loadCitiesAndRoutes();
    showToast('City added successfully', 'success');
}

function showAddRouteModal() {
    const modal = createModal('Add New Route', `
        <form id="addRouteForm">
            <div class="form-group">
                <label>Source City:</label>
                <select id="routeSourceCityAdd" class="form-control" required>
                    ${getCitiesOptions()}
                </select>
            </div>
            <div class="form-group">
                <label>Destination City:</label>
                <select id="routeDestCityAdd" class="form-control" required>
                    ${getCitiesOptions()}
                </select>
            </div>
            <div class="form-group">
                <label>Distance (km):</label>
                <input type="number" id="routeDistance" class="form-control" min="1" required>
            </div>
        </form>
    `, [
        { text: 'Cancel', class: 'btn-secondary', onClick: closeModal },
        { text: 'Add Route', class: 'btn-primary', onClick: addRoute }
    ]);
    
    showModal(modal);
}

function addRoute() {
    const sourceId = parseInt(document.getElementById('routeSourceCityAdd').value);
    const destId = parseInt(document.getElementById('routeDestCityAdd').value);
    const distance = parseInt(document.getElementById('routeDistance').value);
    
    if (!sourceId || !destId || !distance) {
        showToast('Please fill all fields', 'error');
        return;
    }
    
    if (sourceId === destId) {
        showToast('Source and destination must be different', 'error');
        return;
    }
    
    const newRoute = {
        sourceId,
        destId,
        distance,
        blocked: false
    };
    
    systemData.routes.push(newRoute);
    saveDataToJSON();
    
    closeModal();
    loadCitiesAndRoutes();
    showToast('Route added successfully', 'success');
}


function pickupParcel() {
    const pendingParcels = systemData.parcels.filter(p => p.status === 'Pending');
    
    if (pendingParcels.length === 0) {
        showToast('No parcels in pickup queue', 'info');
        return;
    }
    
    const parcel = pendingParcels[0];
    parcel.status = 'PickedUp';
    parcel.history.push(`${new Date().toISOString()} - Picked Up`);
    
    
    setTimeout(() => {
        parcel.status = 'InWarehouse';
        parcel.history.push(`${new Date().toISOString()} - Arrived at Warehouse`);
        saveDataToJSON();
        updateDashboard();
    }, 100);
    
    saveDataToJSON();
    updateDashboard();
    showToast(`Parcel #${parcel.trackingId} picked up`, 'success');
}

function moveToTransit() {
    const warehouseParcels = systemData.parcels.filter(p => p.status === 'InWarehouse');
    
    if (warehouseParcels.length === 0) {
        showToast('No parcels in warehouse', 'info');
        return;
    }
    
    
    warehouseParcels.sort((a, b) => a.priority - b.priority);
    
    const parcel = warehouseParcels[0];
    parcel.status = 'InTransit';
    parcel.history.push(`${new Date().toISOString()} - In Transit`);
    
    saveDataToJSON();
    updateDashboard();
    showToast(`Parcel #${parcel.trackingId} moved to transit`, 'success');
}

function deliverParcel() {
    const transitParcels = systemData.parcels.filter(p => p.status === 'InTransit');
    
    if (transitParcels.length === 0) {
        showToast('No parcels in transit', 'info');
        return;
    }
    
    const modal = createModal('Deliver Parcel', `
        <form id="deliverParcelForm">
            <div class="form-group">
                <label>Select Parcel:</label>
                <select id="deliverParcelId" class="form-control" required>
                    ${transitParcels.map(p => `<option value="${p.trackingId}">#${p.trackingId} - ${p.sender} to ${p.receiver}</option>`).join('')}
                </select>
            </div>
        </form>
    `, [
        { text: 'Cancel', class: 'btn-secondary', onClick: closeModal },
        { text: 'Mark as Delivered', class: 'btn-success', onClick: confirmDelivery }
    ]);
    
    showModal(modal);
}

function confirmDelivery() {
    const trackingId = parseInt(document.getElementById('deliverParcelId').value);
    const parcel = systemData.parcels.find(p => p.trackingId === trackingId);
    
    if (parcel) {
        parcel.status = 'Delivered';
        parcel.history.push(`${new Date().toISOString()} - Delivered`);
        
        saveDataToJSON();
        updateDashboard();
        closeModal();
        showToast(`Parcel #${trackingId} delivered`, 'success');
    }
}

function showAssignRiderModal() {
    if (systemData.riders.length === 0) {
        showToast('No riders available', 'error');
        return;
    }
    
    const inWarehouseParcels = systemData.parcels.filter(p => p.status === 'InWarehouse');
    
    if (inWarehouseParcels.length === 0) {
        showToast('No parcels in warehouse to assign', 'info');
        return;
    }
    
    const modal = createModal('Assign Rider to Parcel', `
        <form id="assignRiderForm">
            <div class="form-group">
                <label>Select Parcel:</label>
                <select id="assignParcelId" class="form-control" required>
                    ${inWarehouseParcels.map(p => `<option value="${p.trackingId}">#${p.trackingId} - ${p.weight}kg</option>`).join('')}
                </select>
            </div>
            <div class="form-group">
                <label>Select Rider:</label>
                <select id="assignRiderId" class="form-control" required>
                    ${systemData.riders.map(r => `<option value="${r.riderId}">${r.name} (${r.currentLoad || 0}/${r.capacity} kg)</option>`).join('')}
                </select>
            </div>
        </form>
    `, [
        { text: 'Cancel', class: 'btn-secondary', onClick: closeModal },
        { text: 'Assign', class: 'btn-primary', onClick: assignRider }
    ]);
    
    showModal(modal);
}

function assignRider() {
    const parcelId = parseInt(document.getElementById('assignParcelId').value);
    const riderId = parseInt(document.getElementById('assignRiderId').value);
    
    const parcel = systemData.parcels.find(p => p.trackingId === parcelId);
    const rider = systemData.riders.find(r => r.riderId === riderId);
    
    if (!parcel || !rider) {
        showToast('Invalid selection', 'error');
        return;
    }
    
    if ((rider.currentLoad || 0) + parcel.weight > rider.capacity) {
        showToast('Rider capacity exceeded', 'error');
        return;
    }
    
    rider.currentLoad = (rider.currentLoad || 0) + parcel.weight;
    rider.assignedParcels = rider.assignedParcels || [];
    rider.assignedParcels.push(parcelId);
    
    parcel.assignedRiderId = riderId;
    parcel.history.push(`${new Date().toISOString()} - Assigned to Rider ${rider.name}`);
    
    saveDataToJSON();
    closeModal();
    showToast(`Parcel #${parcelId} assigned to ${rider.name}`, 'success');
}


function viewParcelStats() {
    const total = systemData.parcels.length;
    const byStatus = {};
    const byPriority = {};
    
    systemData.parcels.forEach(p => {
        byStatus[p.status] = (byStatus[p.status] || 0) + 1;
        byPriority[p.priority] = (byPriority[p.priority] || 0) + 1;
    });
    
    const statusHtml = Object.entries(byStatus).map(([status, count]) => 
        `<tr><td>${status}</td><td>${count}</td><td>${((count/total)*100).toFixed(1)}%</td></tr>`
    ).join('');
    
    const priorityLabels = { 1: 'Overnight', 2: 'Two-Day', 3: 'Normal' };
    const priorityHtml = Object.entries(byPriority).map(([priority, count]) => 
        `<tr><td>${priorityLabels[priority]}</td><td>${count}</td><td>${((count/total)*100).toFixed(1)}%</td></tr>`
    ).join('');
    
    const resultDiv = document.getElementById('reportResult');
    resultDiv.innerHTML = `
        <h3>Parcel Statistics</h3>
        <p><strong>Total Parcels:</strong> ${total}</p>
        
        <h4 style="margin-top: 2rem;">By Status</h4>
        <table class="data-table">
            <thead>
                <tr><th>Status</th><th>Count</th><th>Percentage</th></tr>
            </thead>
            <tbody>${statusHtml}</tbody>
        </table>
        
        <h4 style="margin-top: 2rem;">By Priority</h4>
        <table class="data-table">
            <thead>
                <tr><th>Priority</th><th>Count</th><th>Percentage</th></tr>
            </thead>
            <tbody>${priorityHtml}</tbody>
        </table>
    `;
}

function viewRiderStats() {
    if (systemData.riders.length === 0) {
        const resultDiv = document.getElementById('reportResult');
        resultDiv.innerHTML = '<p class="text-muted">No riders found</p>';
        return;
    }
    
    const html = systemData.riders.map(rider => {
        const utilization = rider.capacity > 0 ? ((rider.currentLoad || 0) / rider.capacity * 100).toFixed(1) : 0;
        const assignedCount = rider.assignedParcels ? rider.assignedParcels.length : 0;
        
        return `
            <tr>
                <td>${rider.name}</td>
                <td>${rider.currentLoad || 0} / ${rider.capacity} kg</td>
                <td>${utilization}%</td>
                <td>${assignedCount}</td>
                <td>${getCityName(rider.currentCityId)}</td>
            </tr>
        `;
    }).join('');
    
    const resultDiv = document.getElementById('reportResult');
    resultDiv.innerHTML = `
        <h3>Rider Performance</h3>
        <table class="data-table">
            <thead>
                <tr>
                    <th>Rider Name</th>
                    <th>Load</th>
                    <th>Utilization</th>
                    <th>Assigned Parcels</th>
                    <th>Location</th>
                </tr>
            </thead>
            <tbody>${html}</tbody>
        </table>
    `;
}

function viewMissingParcels() {
    const missing = systemData.parcels.filter(p => p.status === 'Missing');
    
    if (missing.length === 0) {
        const resultDiv = document.getElementById('reportResult');
        resultDiv.innerHTML = '<p class="text-muted">No missing parcels</p>';
        showToast('No missing parcels found', 'success');
        return;
    }
    
    const html = missing.map(parcel => {
        const sourceCity = getCityName(parcel.sourceCityId);
        const destCity = getCityName(parcel.destCityId);
        
        return `
            <tr>
                <td>#${parcel.trackingId}</td>
                <td>${parcel.sender}</td>
                <td>${parcel.receiver}</td>
                <td>${sourceCity} → ${destCity}</td>
                <td>${parcel.deliveryAttempts}</td>
            </tr>
        `;
    }).join('');
    
    const resultDiv = document.getElementById('reportResult');
    resultDiv.innerHTML = `
        <h3>Missing Parcels</h3>
        <p class="text-muted"><i class="fas fa-exclamation-triangle"></i> ${missing.length} parcel(s) reported as missing</p>
        <table class="data-table">
            <thead>
                <tr>
                    <th>Tracking ID</th>
                    <th>Sender</th>
                    <th>Receiver</th>
                    <th>Route</th>
                    <th>Delivery Attempts</th>
                </tr>
            </thead>
            <tbody>${html}</tbody>
        </table>
    `;
}


function getCityName(cityId) {
    const city = systemData.cities.find(c => c.id === cityId);
    return city ? city.name : `City ${cityId}`;
}

function getCitiesOptions() {
    console.log('getCitiesOptions called, cities count:', systemData.cities.length);
    if (systemData.cities.length === 0) {
        return '<option value="">No cities available</option>';
    }
    return systemData.cities.map(city => 
        `<option value="${city.id}">${city.name}</option>`
    ).join('');
}

function getStatusBadge(status) {
    const badges = {
        'Pending': 'badge-pending',
        'PickedUp': 'badge-pickedup',
        'InWarehouse': 'badge-inwarehouse',
        'InTransit': 'badge-intransit',
        'Delivered': 'badge-delivered',
        'Returned': 'badge-returned',
        'Missing': 'badge-missing'
    };
    return `<span class="badge ${badges[status]}">${status}</span>`;
}

function getPriorityBadge(priority) {
    const labels = {
        1: { text: 'Overnight', class: 'badge-overnight' },
        2: { text: 'Two-Day', class: 'badge-twoday' },
        3: { text: 'Normal', class: 'badge-normal' }
    };
    const badge = labels[priority] || labels[3];
    return `<span class="badge ${badge.class}">${badge.text}</span>`;
}


function createModal(title, body, buttons) {
    const buttonsHtml = buttons.map(btn => 
        `<button class="btn ${btn.class}" onclick="${btn.onClick.name}()">${btn.text}</button>`
    ).join('');
    
    return `
        <div class="modal show">
            <div class="modal-content">
                <div class="modal-header">
                    <h3>${title}</h3>
                    <button class="modal-close" onclick="closeModal()">
                        <i class="fas fa-times"></i>
                    </button>
                </div>
                <div class="modal-body">
                    ${body}
                </div>
                <div class="modal-footer">
                    ${buttonsHtml}
                </div>
            </div>
        </div>
    `;
}

function showModal(html) {
    document.getElementById('modalContainer').innerHTML = html;
}

function closeModal() {
    document.getElementById('modalContainer').innerHTML = '';
}


function showToast(message, type = 'info') {
    const icons = {
        success: 'fa-check-circle',
        error: 'fa-exclamation-circle',
        warning: 'fa-exclamation-triangle',
        info: 'fa-info-circle'
    };
    
    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    toast.innerHTML = `
        <i class="fas ${icons[type]} toast-icon"></i>
        <span class="toast-message">${message}</span>
        <button class="toast-close" onclick="this.parentElement.remove()">
            <i class="fas fa-times"></i>
        </button>
    `;
    
    document.getElementById('toastContainer').appendChild(toast);
    
    setTimeout(() => {
        toast.remove();
    }, 5000);
}


function viewParcelDetails(trackingId) {
    document.getElementById('trackingIdInput').value = trackingId;
    document.querySelector('[data-section="tracking"]').click();
    trackParcel();
}

function editParcel(trackingId) {
    showToast('Edit functionality coming soon', 'info');
}

function viewRiderDetails(riderId) {
    const rider = systemData.riders.find(r => r.riderId === riderId);
    if (!rider) return;
    
    const location = getCityName(rider.currentCityId);
    const assignedParcels = rider.assignedParcels || [];
    const parcelsHtml = assignedParcels.map(pid => {
        const parcel = systemData.parcels.find(p => p.trackingId === pid);
        return parcel ? `<li>Parcel #${pid} - ${parcel.sender} to ${parcel.receiver}</li>` : '';
    }).join('');
    
    const modal = createModal('Rider Details', `
        <p><strong>Rider ID:</strong> ${rider.riderId}</p>
        <p><strong>Name:</strong> ${rider.name}</p>
        <p><strong>Capacity:</strong> ${rider.capacity} kg</p>
        <p><strong>Current Load:</strong> ${rider.currentLoad || 0} kg</p>
        <p><strong>Location:</strong> ${location}</p>
        <p><strong>Assigned Parcels:</strong></p>
        <ul style="margin-left: 1.5rem;">
            ${assignedParcels.length > 0 ? parcelsHtml : '<li>No parcels assigned</li>'}
        </ul>
    `, [
        { text: 'Close', class: 'btn-secondary', onClick: closeModal }
    ]);
    
    showModal(modal);
}
