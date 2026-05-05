export const sensorList = [
	{
		key: 'temperature',
		label: '温度',
		value: 26.8,
		unit: '℃',
		status: '正常',
		icon: '🌡',
		iconPath: '/static/icons/temperature_sensor_sensor_a.png',
		tone: 'blue'
	},
	{
		key: 'humidity',
		label: '湿度',
		value: 63,
		unit: '%RH',
		status: '正常',
		icon: '💧',
		iconPath: '/static/icons/humidity_sensor_sensor_a.png',
		tone: 'teal'
	},
	{
		key: 'pressure',
		label: '大气压强',
		value: 101.3,
		unit: 'kPa',
		status: '正常',
		icon: '⌁',
		iconPath: '/static/icons/air_pressure_sensor_sensor_a.png',
		tone: 'blue'
	},
	{
		key: 'light',
		label: '光照',
		value: 58.5,
		unit: '%',
		status: '正常',
		icon: '☼',
		iconPath: '/static/icons/light_sensor_sensor_a.png',
		tone: 'teal'
	}
]

export const ledState = {
	key: 'led',
	label: 'LED 状态',
	value: true,
	unit: '',
	status: '正常',
	icon: '💡',
	iconPath: '/static/icons/led_status_control_c.png',
	tone: 'warning'
}

export const monitorPoints = [
	...sensorList,
	ledState
]

export const historyMap = {
	temperature: [
		{ time: '09:00', value: '26.4 ℃', status: '正常', level: 42 },
		{ time: '12:00', value: '27.0 ℃', status: '正常', level: 66 },
		{ time: '15:00', value: '26.1 ℃', status: '正常', level: 52 },
		{ time: '18:00', value: '25.3 ℃', status: '正常', level: 38 }
	],
	humidity: [
		{ time: '09:00', value: '61 %RH', status: '正常', level: 58 },
		{ time: '12:00', value: '63 %RH', status: '正常', level: 64 },
		{ time: '15:00', value: '60 %RH', status: '正常', level: 54 },
		{ time: '18:00', value: '62 %RH', status: '正常', level: 60 }
	],
	pressure: [
		{ time: '09:00', value: '101.2 kPa', status: '正常', level: 48 },
		{ time: '12:00', value: '101.3 kPa', status: '正常', level: 56 },
		{ time: '15:00', value: '101.1 kPa', status: '正常', level: 43 },
		{ time: '18:00', value: '101.4 kPa', status: '正常', level: 62 }
	],
	light: [
		{ time: '09:00', value: '52.6 %', status: '正常', level: 44 },
		{ time: '12:00', value: '58.5 %', status: '正常', level: 62 },
		{ time: '15:00', value: '61.2 %', status: '正常', level: 68 },
		{ time: '18:00', value: '47.9 %', status: '正常', level: 38 }
	],
	led: [
		{ time: '09:00', value: '开启', status: '正常', level: 80 },
		{ time: '12:00', value: '关闭', status: '正常', level: 30 },
		{ time: '15:00', value: '开启', status: '正常', level: 80 },
		{ time: '18:00', value: '开启', status: '正常', level: 80 }
	]
}

export const defaultSettings = {
	platform: 'OneNet',
	dataMode: 'onenet',
	connectionStatus: 'untested',
	productId: '45Z51DlJn3',
	token: 'version=2018-10-31&res=products%2F45Z51DlJn3%2Fdevices%2Fstm32-esp32c3&et=1809399480&method=md5&sign=QAfZy6%2F2oq090K%2B043BVrA%3D%3D',
	deviceId: 'stm32-esp32c3',
	serverHost: 'iot-api.heclouds.com',
	reportInterval: '60'
}
