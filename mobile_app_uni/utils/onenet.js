import { sensorList, ledState } from './mockData.js'

const PROPERTY_MAP = {
	temperature: {
		identifier: 'Temp',
		precision: 1
	},
	humidity: {
		identifier: 'Humi',
		precision: 1
	},
	pressure: {
		identifier: 'Press',
		precision: 2
	},
	light: {
		identifier: 'Light',
		precision: 1
	},
	led: {
		identifier: 'led'
	}
}

const UNIT_MAP = sensorList.reduce((result, item) => {
	result[item.key] = item.unit || ''
	return result
}, {
	led: ''
})

function getBaseUrl(settings) {
	const host = (settings && settings.serverHost) || 'iot-api.heclouds.com'
	if (/^https?:\/\//.test(host)) {
		return host.replace(/\/$/, '')
	}
	return `https://${host}`
}

function getDeviceName(settings) {
	return settings.deviceName || settings.deviceId || ''
}

function normalizeValue(value) {
	if (value && typeof value === 'object' && Object.prototype.hasOwnProperty.call(value, 'value')) {
		return value.value
	}
	return value
}

function parseBoolean(value) {
	const nextValue = normalizeValue(value)
	if (typeof nextValue === 'boolean') return nextValue
	if (typeof nextValue === 'number') return nextValue !== 0
	if (typeof nextValue === 'string') return nextValue.toLowerCase() === 'true' || nextValue === '1'
	return Boolean(nextValue)
}

function formatMetric(value, precision) {
	if (value === null || value === undefined || value === '') return '--'
	const numericValue = Number(normalizeValue(value))
	return Number.isFinite(numericValue) ? numericValue.toFixed(precision) : String(value)
}

function formatTime(timestamp) {
	const date = new Date(Number(timestamp))
	if (!Number.isFinite(date.getTime())) return '--'
	const pad = (value) => String(value).padStart(2, '0')
	return `${pad(date.getHours())}:${pad(date.getMinutes())}:${pad(date.getSeconds())}`
}

function getNumericLevel(value) {
	const numericValue = Number.parseFloat(String(value).replace(/[^\d.-]/g, ''))
	if (!Number.isFinite(numericValue)) {
		return parseBoolean(value) ? 82 : 28
	}
	return Math.max(12, Math.min(88, numericValue))
}

function request(settings, options) {
	return new Promise((resolve, reject) => {
		uni.request({
			url: `${getBaseUrl(settings)}${options.path}`,
			method: options.method || 'GET',
			data: options.data || {},
			header: {
				// OneNet 真实鉴权方式以控制台文档为准，当前沿用已验证的 authorization Token 请求头。
				authorization: settings.token || ''
			},
			success: (res) => {
				if (!res || !res.data) {
					reject(new Error('OneNet 返回为空。'))
					return
				}
				if (res.data.code !== 0) {
					reject(new Error(res.data.msg || 'OneNet 请求失败。'))
					return
				}
				resolve(res.data)
			},
			fail: (error) => {
				reject(new Error(error && error.errMsg ? error.errMsg : 'OneNet 网络请求失败。'))
			}
		})
	})
}

export function validateSettings(settings) {
	if (!settings || !settings.productId || !getDeviceName(settings) || !settings.serverHost) {
		throw new Error('请先填写 Product ID、Device ID 和服务器地址。')
	}
	if (settings.dataMode === 'onenet' && !settings.token) {
		throw new Error('OneNet 模式需要填写 Token。')
	}
}

export function testConnection(settings) {
	return new Promise((resolve, reject) => {
		try {
			if (!settings || settings.dataMode !== 'onenet') {
				resolve({
					connected: true,
					mode: 'mock'
				})
				return
			}

			validateSettings(settings)
			queryDeviceProperties(settings)
				.then(() => {
					resolve({
						connected: true,
						mode: 'onenet'
					})
				})
				.catch(reject)
		} catch (error) {
			reject(error)
		}
	})
}

export function queryDeviceProperties(settings) {
	validateSettings(settings)
	return request(settings, {
		// OneNet 真实 API 地址以控制台文档为准；此处保留旧版已验证的物模型属性查询路径。
		path: '/thingmodel/query-device-property',
		method: 'GET',
		data: {
			product_id: settings.productId,
			device_name: getDeviceName(settings)
		}
	}).then((body) => {
		if (!Array.isArray(body.data)) {
			throw new Error('设备属性返回格式不符合预期。')
		}
		return body.data
	})
}

export function setDeviceProperty(settings, params) {
	validateSettings(settings)
	return request(settings, {
		// OneNet 真实 API 地址以控制台文档为准；此处保留旧版已验证的物模型属性设置路径。
		path: '/thingmodel/set-device-property',
		method: 'POST',
		data: {
			product_id: settings.productId,
			device_name: getDeviceName(settings),
			params
		}
	})
}

export function queryPropertyHistory(settings, identifier, range = {}) {
	validateSettings(settings)
	return request(settings, {
		// OneNet 历史属性真实 API 路径以控制台文档为准；这里保留适配入口。
		// 如果控制台接口参数与此处不同，只需要调整这一层，历史页无需改动。
		path: '/thingmodel/query-device-property-history',
		method: 'GET',
		data: {
			product_id: settings.productId,
			device_name: getDeviceName(settings),
			identifier,
			start_time: range.startTime || '',
			end_time: range.endTime || ''
		}
	}).then((body) => {
		if (body.data && Array.isArray(body.data.list)) {
			return body.data.list
		}
		if (Array.isArray(body.data)) {
			return body.data
		}
		return []
	})
}

export function getPropertyIdentifier(key) {
	return PROPERTY_MAP[key] ? PROPERTY_MAP[key].identifier : ''
}

export function buildHistoryRows(key, rows) {
	const config = PROPERTY_MAP[key]
	const unit = UNIT_MAP[key] || ''
	return rows
		.slice()
		.reverse()
		.map((row) => {
			const rawValue = normalizeValue(row.value)
			const displayValue = key === 'led'
				? (parseBoolean(rawValue) ? '开启' : '关闭')
				: formatMetric(rawValue, config ? config.precision : 1)
			return {
				time: formatTime(row.time),
				value: unit ? `${displayValue} ${unit}` : displayValue,
				status: '正常',
				level: getNumericLevel(rawValue)
			}
		})
}

export function buildMockMonitorData() {
	return {
		sensors: sensorList.map((item) => ({ ...item })),
		ledOn: Boolean(ledState.value),
		errorMessage: ''
	}
}

export function buildOnenetMonitorData(items) {
	const findItem = (identifier) => items.find((entry) => entry.identifier === identifier)
	const sensors = sensorList.map((sensor) => {
		const config = PROPERTY_MAP[sensor.key]
		const item = config ? findItem(config.identifier) : null
		return {
			...sensor,
			value: item ? formatMetric(item.value, config.precision) : '--',
			status: item ? '正常' : '未配置'
		}
	})
	const ledItem = findItem(PROPERTY_MAP.led.identifier)
	return {
		sensors,
		ledOn: ledItem ? parseBoolean(ledItem.value) : false,
		errorMessage: ''
	}
}
