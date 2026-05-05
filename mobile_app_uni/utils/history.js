import { historyMap } from './mockData.js'

const HISTORY_KEY = 'iot_control_system_history'
const MAX_ROWS = 80

function pad(value) {
	return String(value).padStart(2, '0')
}

function nowTime() {
	const date = new Date()
	return `${pad(date.getHours())}:${pad(date.getMinutes())}:${pad(date.getSeconds())}`
}

function getNumericLevel(value) {
	const numericValue = Number.parseFloat(String(value).replace(/[^\d.-]/g, ''))
	if (!Number.isFinite(numericValue)) {
		return String(value) === '开启' ? 82 : 28
	}
	return Math.max(12, Math.min(88, numericValue))
}

function normalizeRow(row) {
	return {
		time: row.time || nowTime(),
		value: row.value || '--',
		status: row.status || '正常',
		level: row.level || getNumericLevel(row.value)
	}
}

export function loadLocalHistory() {
	try {
		const saved = uni.getStorageSync(HISTORY_KEY)
		if (saved && typeof saved === 'object') {
			return saved
		}
	} catch (error) {
		console.warn('Failed to load local history', error)
	}
	return {}
}

export function saveLocalHistory(history) {
	try {
		uni.setStorageSync(HISTORY_KEY, history)
	} catch (error) {
		console.warn('Failed to save local history', error)
	}
	return history
}

export function appendHistorySnapshot({ sensors = [], ledOn = false }) {
	const history = loadLocalHistory()
	const time = nowTime()

	sensors.forEach((sensor) => {
		if (!sensor || !sensor.key || sensor.value === '--') return
		const value = `${sensor.value}${sensor.unit ? ` ${sensor.unit}` : ''}`
		const rows = history[sensor.key] || []
		const lastRow = rows[rows.length - 1]
		if (lastRow && lastRow.time === time && lastRow.value === value) return
		history[sensor.key] = [
			...rows,
			normalizeRow({
				time,
				value,
				status: sensor.status || '正常',
				level: getNumericLevel(sensor.value)
			})
		].slice(-MAX_ROWS)
	})

	const ledRows = history.led || []
	const ledValue = ledOn ? '开启' : '关闭'
	history.led = [
		...ledRows,
		normalizeRow({
			time,
			value: ledValue,
			status: '正常',
			level: ledOn ? 82 : 28
		})
	].slice(-MAX_ROWS)

	return saveLocalHistory(history)
}

export function loadHistoryRows(key, useMockFallback = true) {
	const history = loadLocalHistory()
	const rows = history[key] || []
	if (rows.length > 0) {
		return rows
	}
	return useMockFallback ? (historyMap[key] || []) : []
}

export function loadHistoryMap(useMockFallback = true) {
	const history = loadLocalHistory()
	if (!useMockFallback) {
		return history
	}
	return {
		...historyMap,
		...history
	}
}
