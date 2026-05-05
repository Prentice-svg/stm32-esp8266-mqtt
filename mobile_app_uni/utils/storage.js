import { defaultSettings } from './mockData.js'

const SETTINGS_KEY = 'iot_control_system_settings'

export function loadSettings() {
	try {
		const saved = uni.getStorageSync(SETTINGS_KEY)
		if (saved && typeof saved === 'object') {
			const nextSettings = {
				...defaultSettings,
				...saved
			}
			if (!nextSettings.token) {
				nextSettings.token = defaultSettings.token
			}
			return {
				...nextSettings
			}
		}
	} catch (error) {
		console.warn('Failed to load settings', error)
	}
	return { ...defaultSettings }
}

export function saveSettings(settings) {
	const nextSettings = {
		...defaultSettings,
		...settings,
		dataMode: settings.dataMode === 'onenet' ? 'onenet' : 'mock',
		connectionStatus: settings.connectionStatus || 'untested'
	}
	uni.setStorageSync(SETTINGS_KEY, nextSettings)
	return nextSettings
}
