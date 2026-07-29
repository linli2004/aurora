.pragma library

const goldenAngle = Math.PI * (3.0 - Math.sqrt(5.0))
const minimumSphereNodeCount = 24
const maximumSphereNodeCount = 24

function clamp(value, minimum, maximum) {
    return Math.max(minimum, Math.min(maximum, value))
}

function hashText(value) {
    let hash = 2166136261
    const text = String(value || "")
    for (let index = 0; index < text.length; ++index) {
        hash ^= text.charCodeAt(index)
        hash = Math.imul(hash, 16777619)
    }
    return hash >>> 0
}

function keyForTrack(track, index) {
    if (!track)
        return "node:" + index
    if (track.key && track.key.length > 0)
        return track.key
    if (track.sourceId && track.sourceId.length > 0)
        return track.sourceId
    if (track.cacheCatalogKey && track.cacheCatalogKey.length > 0)
        return track.cacheCatalogKey
    if (track.source && track.songId)
        return track.source + ":" + track.songId
    if (track.url && track.url.length > 0)
        return track.url
    if (track.trackId && track.trackId.length > 0)
        return track.trackId
    return "node:" + index + ":" + (track.title || "")
}

function trackTitle(track, fallback) {
    if (track && track.title && track.title.length > 0)
        return track.title
    return fallback
}

function normalizeVector(x, y, z) {
    const length = Math.max(0.001, Math.sqrt(x * x + y * y + z * z))
    return {
        x: x / length,
        y: y / length,
        z: z / length
    }
}

function vectorDistanceSquared(left, right) {
    const dx = left.sx - right.sx
    const dy = left.sy - right.sy
    const dz = left.sz - right.sz
    return dx * dx + dy * dy + dz * dz
}

function mergeRequiredTracks(tracks, currentTrack, targetTrack) {
    const result = []
    const seen = {}

    function append(track, forcedKey) {
        if (!track)
            return
        const key = forcedKey || keyForTrack(track, result.length)
        if (seen[key])
            return
        const copy = {}
        for (const propertyName in track)
            copy[propertyName] = track[propertyName]
        copy.key = key
        copy.title = trackTitle(copy, "Track " + (result.length + 1))
        result.push(copy)
        seen[key] = true
    }

    append(currentTrack, keyForTrack(currentTrack, 0))
    for (let index = 0; index < tracks.length; ++index)
        append(tracks[index], keyForTrack(tracks[index], index + 1))
    append(targetTrack, keyForTrack(targetTrack, result.length + 1))

    return result
}

function prepareNodes(tracks, currentTrack, targetTrack, width, height) {
    const currentKey = keyForTrack(currentTrack, 0)
    const targetKey = keyForTrack(targetTrack, 1)
    let merged = mergeRequiredTracks(tracks || [], currentTrack, targetTrack)
    if (merged.length > maximumSphereNodeCount) {
        const compact = []
        const seen = {}

        function appendCompact(track) {
            if (!track || seen[track.key] || compact.length >= maximumSphereNodeCount)
                return
            seen[track.key] = true
            compact.push(track)
        }

        appendCompact(merged.find(track => track.key === currentKey))
        appendCompact(merged.find(track => track.key === targetKey))
        for (let index = 0; index < merged.length; ++index)
            appendCompact(merged[index])
        merged = compact
    }

    const desiredCount = merged.length < 10
            ? minimumSphereNodeCount
            : clamp(merged.length, 12, maximumSphereNodeCount)
    const fieldSeed = hashText(currentKey + "|" + targetKey).toString(16)
    while (merged.length < desiredCount) {
        const index = merged.length
        merged.push({
            key: "network-field:" + fieldSeed + ":" + index,
            title: "",
            virtual: true
        })
    }

    const count = Math.max(1, merged.length)
    const usableWidth = Math.max(320, width)
    const usableHeight = Math.max(260, height)
    const radius = Math.min(usableWidth, usableHeight) * 0.33
    const centerX = usableWidth * 0.50
    const centerY = usableHeight * 0.48
    const seedPhase = (hashText(currentKey + targetKey) % 628) / 100.0

    const nodes = []
    for (let index = 0; index < merged.length; ++index) {
        const track = merged[index]
        const key = keyForTrack(track, index)
        const hashed = hashText(key)
        const phase = index * goldenAngle + seedPhase + (hashed % 90) * Math.PI / 540.0
        const t = count === 1 ? 0.0 : (index + 0.5) / count
        let sz = 1.0 - 2.0 * t
        let ring = Math.sqrt(Math.max(0.0, 1.0 - sz * sz))
        let sx = Math.cos(phase) * ring
        let sy = Math.sin(phase) * ring
        let fixed = false

        if (key === currentKey) {
            const vector = normalizeVector(-0.78, 0.18, 0.58)
            sx = vector.x
            sy = vector.y
            sz = vector.z
            fixed = true
        } else if (key === targetKey) {
            const vector = normalizeVector(0.76, -0.10, 0.64)
            sx = vector.x
            sy = vector.y
            sz = vector.z
            fixed = true
        }

        const depth = sz
        const perspective = 0.80 + depth * 0.10
        const x = centerX + sx * radius * 1.18 * perspective
        const y = centerY + sy * radius * 0.76 * perspective

        nodes.push({
            key,
            title: track.virtual === true ? "" : trackTitle(track, "Track " + (index + 1)),
            url: track.url || "",
            trackId: track.trackId || "",
            sourceId: track.sourceId || "",
            cacheCatalogKey: track.cacheCatalogKey || "",
            virtual: track.virtual === true,
            sx,
            sy,
            sz,
            x: clamp(x, usableWidth * 0.10, usableWidth * 0.90),
            y: clamp(y, usableHeight * 0.14, usableHeight * 0.86),
            z: depth,
            fixed,
            current: key === currentKey,
            target: key === targetKey,
            order: index
        })
    }

    return nodes
}

function buildTravelPath(nodes, currentKey, targetKey) {
    const byKey = {}
    for (let index = 0; index < nodes.length; ++index)
        byKey[nodes[index].key] = nodes[index]

    const current = byKey[currentKey]
    const target = byKey[targetKey]
    if (!current || !target || current.key === target.key)
        return current ? [current.key] : []

    const path = [current.key]
    const used = {}
    used[current.key] = true
    used[target.key] = true

    const candidateCount = nodes.length >= 20 ? 5 : 4
    for (let step = 1; step <= candidateCount; ++step) {
        const amount = step / (candidateCount + 1)
        const guide = normalizeVector(
                    current.sx * (1.0 - amount) + target.sx * amount,
                    current.sy * (1.0 - amount) + target.sy * amount,
                    current.sz * (1.0 - amount) + target.sz * amount)
        let bestNode = null
        let bestScore = Number.POSITIVE_INFINITY

        for (let index = 0; index < nodes.length; ++index) {
            const node = nodes[index]
            if (used[node.key])
                continue
            const dx = node.sx - guide.x
            const dy = node.sy - guide.y
            const dz = node.sz - guide.z
            const score = dx * dx + dy * dy + dz * dz
                    + (node.virtual === true ? 0.0 : 0.08)
            if (score < bestScore) {
                bestScore = score
                bestNode = node
            }
        }

        if (bestNode) {
            used[bestNode.key] = true
            path.push(bestNode.key)
        }
    }

    path.push(target.key)
    return path
}

function buildSegments(nodes, currentKey, targetKey) {
    const segments = []
    const seen = {}

    function add(a, b, active) {
        if (!a || !b || a.key === b.key)
            return
        const key = a.key < b.key ? a.key + "|" + b.key : b.key + "|" + a.key
        if (seen[key]) {
            if (active === true)
                seen[key].active = true
            return
        }
        const segment = {
            from: a.key,
            to: b.key,
            active: active === true
        }
        seen[key] = segment
        segments.push(segment)
    }

    const byKey = {}
    for (let index = 0; index < nodes.length; ++index)
        byKey[nodes[index].key] = nodes[index]

    for (let index = 0; index < nodes.length; ++index) {
        const node = nodes[index]
        const nearest = nodes
            .filter(candidate => candidate.key !== node.key)
            .map(candidate => {
                return {
                    node: candidate,
                    distance: vectorDistanceSquared(node, candidate)
                }
            })
            .sort((left, right) => left.distance - right.distance)
        for (let nearestIndex = 0; nearestIndex < Math.min(3, nearest.length); ++nearestIndex)
            add(node, nearest[nearestIndex].node, false)
    }

    const travelPath = buildTravelPath(nodes, currentKey, targetKey)
    for (let index = 0; index < travelPath.length - 1; ++index) {
        add(byKey[travelPath[index]], byKey[travelPath[index + 1]], true)
    }

    return segments
}
