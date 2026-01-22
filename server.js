const express = require('express');
const cors = require('cors');
const fs = require('fs').promises;
const path = require('path');

const app = express();
const PORT = 3000;


app.use(cors());
app.use(express.json());
app.use(express.static('web'));


const DATA_DIR = path.join(__dirname, 'data');
const FILES = {
    cities: path.join(DATA_DIR, 'cities.json'),
    routes: path.join(DATA_DIR, 'routes.json'),
    parcels: path.join(DATA_DIR, 'parcels.json'),
    riders: path.join(DATA_DIR, 'riders.json'),
    admins: path.join(DATA_DIR, 'admins.json')
};


async function readJSON(filePath) {
    try {
        const data = await fs.readFile(filePath, 'utf8');
        return JSON.parse(data);
    } catch (error) {
        console.error(`Error reading ${filePath}:`, error);
        return null;
    }
}


async function writeJSON(filePath, data) {
    try {
        await fs.writeFile(filePath, JSON.stringify(data, null, 2), 'utf8');
        return true;
    } catch (error) {
        console.error(`Error writing ${filePath}:`, error);
        return false;
    }
}





app.post('/api/auth/login', async (req, res) => {
    const { username, password } = req.body;
    const admins = await readJSON(FILES.admins);
    
    const admin = admins.admins.find(a => a.username === username && a.password === password);
    
    if (admin) {
        res.json({ success: true, username });
    } else {
        res.status(401).json({ success: false, message: 'Invalid credentials' });
    }
});





app.get('/api/cities', async (req, res) => {
    const data = await readJSON(FILES.cities);
    res.json(data);
});

app.post('/api/cities', async (req, res) => {
    const { name } = req.body;
    const data = await readJSON(FILES.cities);
    
    const maxId = Math.max(...data.cities.map(c => c.id), 0);
    const newCity = {
        id: maxId + 1,
        name
    };
    
    data.cities.push(newCity);
    await writeJSON(FILES.cities, data);
    
    res.json({ success: true, city: newCity });
});





app.get('/api/routes', async (req, res) => {
    const data = await readJSON(FILES.routes);
    res.json(data);
});

app.post('/api/routes', async (req, res) => {
    const { sourceId, destId, distance } = req.body;
    const data = await readJSON(FILES.routes);
    
    const newRoute = {
        sourceId,
        destId,
        distance,
        blocked: false
    };
    
    data.routes.push(newRoute);
    await writeJSON(FILES.routes, data);
    
    res.json({ success: true, route: newRoute });
});

app.put('/api/routes/block', async (req, res) => {
    const { sourceId, destId, blocked } = req.body;
    const data = await readJSON(FILES.routes);
    
    const route = data.routes.find(r => 
        (r.sourceId === sourceId && r.destId === destId) ||
        (r.sourceId === destId && r.destId === sourceId)
    );
    
    if (route) {
        route.blocked = blocked;
        await writeJSON(FILES.routes, data);
        res.json({ success: true, route });
    } else {
        res.status(404).json({ success: false, message: 'Route not found' });
    }
});





app.get('/api/parcels', async (req, res) => {
    const data = await readJSON(FILES.parcels);
    res.json(data);
});

app.post('/api/parcels', async (req, res) => {
    const { sender, receiver, priority, weight, sourceCityId, destCityId, isFragile } = req.body;
    const data = await readJSON(FILES.parcels);
    
    const newParcel = {
        trackingId: data.nextTrackingId++,
        sender,
        receiver,
        priority: parseInt(priority),
        weight: parseFloat(weight),
        status: 'Pending',
        sourceCityId: parseInt(sourceCityId),
        destCityId: parseInt(destCityId),
        currentCityId: parseInt(sourceCityId),
        deliveryAttempts: 0,
        isFragile: Boolean(isFragile),
        history: [`${new Date().toISOString()} - Parcel Created`],
        currentRoute: []
    };
    
    data.parcels.push(newParcel);
    await writeJSON(FILES.parcels, data);
    
    res.json({ success: true, parcel: newParcel });
});

app.get('/api/parcels/:id', async (req, res) => {
    const trackingId = parseInt(req.params.id);
    const data = await readJSON(FILES.parcels);
    
    const parcel = data.parcels.find(p => p.trackingId === trackingId);
    
    if (parcel) {
        res.json({ success: true, parcel });
    } else {
        res.status(404).json({ success: false, message: 'Parcel not found' });
    }
});

app.put('/api/parcels/:id', async (req, res) => {
    const trackingId = parseInt(req.params.id);
    const updates = req.body;
    const data = await readJSON(FILES.parcels);
    
    const index = data.parcels.findIndex(p => p.trackingId === trackingId);
    
    if (index !== -1) {
        data.parcels[index] = { ...data.parcels[index], ...updates };
        
        
        if (updates.status) {
            data.parcels[index].history.push(`${new Date().toISOString()} - Status changed to ${updates.status}`);
        }
        
        await writeJSON(FILES.parcels, data);
        res.json({ success: true, parcel: data.parcels[index] });
    } else {
        res.status(404).json({ success: false, message: 'Parcel not found' });
    }
});

app.delete('/api/parcels/:id', async (req, res) => {
    const trackingId = parseInt(req.params.id);
    const data = await readJSON(FILES.parcels);
    
    const index = data.parcels.findIndex(p => p.trackingId === trackingId);
    
    if (index !== -1) {
        data.parcels.splice(index, 1);
        await writeJSON(FILES.parcels, data);
        res.json({ success: true });
    } else {
        res.status(404).json({ success: false, message: 'Parcel not found' });
    }
});





app.get('/api/riders', async (req, res) => {
    const data = await readJSON(FILES.riders);
    res.json(data);
});

app.post('/api/riders', async (req, res) => {
    const { name, capacity, currentCityId } = req.body;
    const data = await readJSON(FILES.riders);
    
    const newRider = {
        riderId: data.nextRiderId++,
        name,
        capacity: parseInt(capacity),
        currentCityId: parseInt(currentCityId),
        currentLoad: 0,
        assignedParcels: []
    };
    
    data.riders.push(newRider);
    await writeJSON(FILES.riders, data);
    
    res.json({ success: true, rider: newRider });
});

app.put('/api/riders/:id', async (req, res) => {
    const riderId = parseInt(req.params.id);
    const updates = req.body;
    const data = await readJSON(FILES.riders);
    
    const index = data.riders.findIndex(r => r.riderId === riderId);
    
    if (index !== -1) {
        data.riders[index] = { ...data.riders[index], ...updates };
        await writeJSON(FILES.riders, data);
        res.json({ success: true, rider: data.riders[index] });
    } else {
        res.status(404).json({ success: false, message: 'Rider not found' });
    }
});

app.delete('/api/riders/:id', async (req, res) => {
    const riderId = parseInt(req.params.id);
    const data = await readJSON(FILES.riders);
    
    const index = data.riders.findIndex(r => r.riderId === riderId);
    
    if (index !== -1) {
        data.riders.splice(index, 1);
        await writeJSON(FILES.riders, data);
        res.json({ success: true });
    } else {
        res.status(404).json({ success: false, message: 'Rider not found' });
    }
});





app.post('/api/operations/pickup', async (req, res) => {
    const data = await readJSON(FILES.parcels);
    
    const pendingParcels = data.parcels.filter(p => p.status === 'Pending');
    
    if (pendingParcels.length === 0) {
        return res.json({ success: false, message: 'No parcels in pickup queue' });
    }
    
    const parcel = pendingParcels[0];
    const index = data.parcels.findIndex(p => p.trackingId === parcel.trackingId);
    
    data.parcels[index].status = 'InWarehouse';
    data.parcels[index].history.push(`${new Date().toISOString()} - Picked Up and Moved to Warehouse`);
    
    await writeJSON(FILES.parcels, data);
    
    res.json({ success: true, parcel: data.parcels[index] });
});

app.post('/api/operations/transit', async (req, res) => {
    const { trackingId } = req.body;
    const data = await readJSON(FILES.parcels);
    
    const index = data.parcels.findIndex(p => p.trackingId === trackingId);
    
    if (index === -1) {
        return res.status(404).json({ success: false, message: 'Parcel not found' });
    }
    
    if (data.parcels[index].status !== 'InWarehouse') {
        return res.json({ success: false, message: 'Parcel is not in warehouse' });
    }
    
    data.parcels[index].status = 'InTransit';
    data.parcels[index].history.push(`${new Date().toISOString()} - Moved to Transit`);
    
    await writeJSON(FILES.parcels, data);
    
    res.json({ success: true, parcel: data.parcels[index] });
});

app.post('/api/operations/deliver', async (req, res) => {
    const { trackingId } = req.body;
    const data = await readJSON(FILES.parcels);
    
    const index = data.parcels.findIndex(p => p.trackingId === trackingId);
    
    if (index === -1) {
        return res.status(404).json({ success: false, message: 'Parcel not found' });
    }
    
    data.parcels[index].status = 'Delivered';
    data.parcels[index].history.push(`${new Date().toISOString()} - Delivered`);
    
    await writeJSON(FILES.parcels, data);
    
    res.json({ success: true, parcel: data.parcels[index] });
});

app.post('/api/operations/assign-rider', async (req, res) => {
    const { parcelId, riderId } = req.body;
    
    const parcelsData = await readJSON(FILES.parcels);
    const ridersData = await readJSON(FILES.riders);
    
    const parcelIndex = parcelsData.parcels.findIndex(p => p.trackingId === parcelId);
    const riderIndex = ridersData.riders.findIndex(r => r.riderId === riderId);
    
    if (parcelIndex === -1 || riderIndex === -1) {
        return res.status(404).json({ success: false, message: 'Parcel or Rider not found' });
    }
    
    const parcel = parcelsData.parcels[parcelIndex];
    const rider = ridersData.riders[riderIndex];
    
    if ((rider.currentLoad || 0) + parcel.weight > rider.capacity) {
        return res.json({ success: false, message: 'Rider capacity exceeded' });
    }
    
    rider.currentLoad = (rider.currentLoad || 0) + parcel.weight;
    rider.assignedParcels = rider.assignedParcels || [];
    rider.assignedParcels.push(parcelId);
    
    parcel.assignedRiderId = riderId;
    parcel.history.push(`${new Date().toISOString()} - Assigned to Rider ${rider.name}`);
    
    await writeJSON(FILES.parcels, parcelsData);
    await writeJSON(FILES.riders, ridersData);
    
    res.json({ success: true, parcel, rider });
});





app.get('/api/stats', async (req, res) => {
    const parcelsData = await readJSON(FILES.parcels);
    const ridersData = await readJSON(FILES.riders);
    
    const stats = {
        totalParcels: parcelsData.parcels.length,
        byStatus: {},
        byPriority: {},
        totalRiders: ridersData.riders.length,
        queues: {
            pickup: 0,
            warehouse: 0,
            transit: 0
        }
    };
    
    parcelsData.parcels.forEach(p => {
        stats.byStatus[p.status] = (stats.byStatus[p.status] || 0) + 1;
        stats.byPriority[p.priority] = (stats.byPriority[p.priority] || 0) + 1;
        
        if (p.status === 'Pending') stats.queues.pickup++;
        else if (p.status === 'InWarehouse') stats.queues.warehouse++;
        else if (p.status === 'InTransit') stats.queues.transit++;
    });
    
    res.json(stats);
});





app.listen(PORT, () => {
    console.log(`
╔════════════════════════════════════════════╗
║   SwiftEx Courier System - Web Server     ║
╠════════════════════════════════════════════╣
║   Server running on:                       ║
║   http:
║                                            ║
║   API Endpoints:                           ║
║   - /api/cities                            ║
║   - /api/routes                            ║
║   - /api/parcels                           ║
║   - /api/riders                            ║
║   - /api/operations/*                      ║
║   - /api/stats                             ║
╚════════════════════════════════════════════╝
    `);
});
