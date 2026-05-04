<template>
	<view class="page">
		<view class="page-title">
			<view class="title-main">IoT 环境监测终端</view>
			<view class="title-sub">OneNet Device Dashboard</view>
		</view>

		<view class="card-grid">
			<view
				v-for="card in dashboardCards"
				:key="card.key"
				class="dashboard-card"
			>
				<image class="card-icon" :src="card.icon" mode="aspectFit" />
				<view class="card-label">{{ card.label }}</view>
				<view class="card-divider"></view>
				<view class="card-value-row">
					<text class="card-value" :class="card.tone">{{ card.value }}</text>
					<text v-if="card.unit" class="card-unit">{{ card.unit }}</text>
				</view>
			</view>

			<view class="dashboard-card control-card">
				<image class="card-icon" src="/static/icons/led_status_control_c.png" mode="aspectFit" />
				<view class="card-label">板载 LED</view>
				<view class="card-divider"></view>
				<view class="card-value-row control-value-row">
					<text class="card-value" :class="LED ? 'tone-green' : 'tone-gray'">
						{{ LED ? '开启' : '关闭' }}
					</text>
					<switch class="led-switch" :checked="LED" :disabled="ledSubmitting" @change="onLEDSwitch" color="#11823b" />
				</view>
			</view>
		</view>

		<view class="status-grid">
			<view class="status-card">
				<text class="status-label">连接状态</text>
				<text class="status-text" :class="{ online: !loading && !errorMessage, error: !!errorMessage }">
					{{ statusText }}
				</text>
			</view>
			<view class="status-card">
				<text class="status-label">最近同步</text>
				<text class="status-text tone-blue">{{ lastSyncText }}</text>
			</view>
		</view>

		<view v-if="errorMessage" class="error-box">
			<text class="error-title">请求失败</text>
			<text class="error-text">{{ errorMessage }}</text>
		</view>
	</view>
</template>

<script>
export default {
	data() {
		return {
			humidity: null,
			temperature: null,
			light: null,
			pressure: null,
			LED: false,
			loading: false,
			ledSubmitting: false,
			errorMessage: '',
			lastSyncText: '--',
			pollingTimer: null,
			pollingIntervalMs: 2000,
			apiConfig: {
				productId: '45Z51DlJn3',
				deviceName: 'stm32-esp32c3',
				token: 'version=2018-10-31&res=products%2F45Z51DlJn3%2Fdevices%2Fstm32-esp32c3&et=1809399480&method=md5&sign=QAfZy6%2F2oq090K%2B043BVrA%3D%3D'
			}
		}
	},
	computed: {
		humidityDisplay() {
			return this.formatMetric(this.humidity, 1)
		},
		temperatureDisplay() {
			return this.formatMetric(this.temperature, 1)
		},
		lightDisplay() {
			return this.formatMetric(this.light, 1)
		},
		pressureDisplay() {
			return this.formatMetric(this.pressure, 2)
		},
		dashboardCards() {
			return [
				{
					key: 'temp',
					label: '温度',
					value: this.temperatureDisplay,
					unit: '°C',
					icon: '/static/icons/temperature_sensor_sensor_a.png',
					tone: 'tone-blue'
				},
				{
					key: 'humi',
					label: '湿度',
					value: this.humidityDisplay,
					unit: '%RH',
					icon: '/static/icons/humidity_sensor_sensor_a.png',
					tone: 'tone-blue'
				},
				{
					key: 'press',
					label: '大气压强',
					value: this.pressureDisplay,
					unit: 'hPa',
					icon: '/static/icons/air_pressure_sensor_sensor_a.png',
					tone: 'tone-blue'
				},
				{
					key: 'light',
					label: '光照',
					value: this.lightDisplay,
					unit: '%',
					icon: '/static/icons/light_sensor_sensor_a.png',
					tone: 'tone-orange'
				}
			]
		},
		statusText() {
			if (this.errorMessage) {
				return '异常'
			}
			if (this.loading) {
				return '同步中'
			}
			return '已连接'
		}
	},
	onShow() {
		this.fetchDeviceData()
		this.startPolling()
	},
	onHide() {
		this.stopPolling()
	},
	onUnload() {
		this.stopPolling()
	},
	methods: {
		formatMetric(value, digits) {
			if (value === null || value === undefined || value === '--' || value === '') {
				return '--'
			}

			const numericValue = Number(value)
			return Number.isFinite(numericValue) ? numericValue.toFixed(digits) : String(value)
		},
		formatTime(timestamp) {
			if (!timestamp || !Number.isFinite(Number(timestamp))) {
				return '--'
			}

			const date = new Date(Number(timestamp))
			const pad = (value) => String(value).padStart(2, '0')
			return `${pad(date.getHours())}:${pad(date.getMinutes())}:${pad(date.getSeconds())}`
		},
		startPolling() {
			this.stopPolling()
			this.pollingTimer = setInterval(() => {
				this.fetchDeviceData()
			}, this.pollingIntervalMs)
		},
		stopPolling() {
			if (this.pollingTimer) {
				clearInterval(this.pollingTimer)
				this.pollingTimer = null
			}
		},
		getPropertyItem(items, identifier) {
			return items.find((entry) => entry.identifier === identifier) || null
		},
		normalizeValue(value) {
			if (value && typeof value === 'object' && 'value' in value) {
				return value.value
			}
			return value
		},
		parseBoolean(value) {
			const normalizedValue = this.normalizeValue(value)
			if (typeof normalizedValue === 'boolean') {
				return normalizedValue
			}
			if (typeof normalizedValue === 'string') {
				return normalizedValue.toLowerCase() === 'true'
			}
			if (typeof normalizedValue === 'number') {
				return normalizedValue !== 0
			}
			return Boolean(normalizedValue)
		},
		getApiMessage(response, fallbackMessage) {
			const message = response && response.data && response.data.msg
			return message || fallbackMessage
		},
		updateDeviceState(items) {
			const humidityItem = this.getPropertyItem(items, 'Humi')
			const temperatureItem = this.getPropertyItem(items, 'Temp')
			const lightItem = this.getPropertyItem(items, 'Light')
			const pressureItem = this.getPropertyItem(items, 'Press')
			const ledItem = this.getPropertyItem(items, 'led')

			this.humidity = humidityItem ? this.normalizeValue(humidityItem.value) : null
			this.temperature = temperatureItem ? this.normalizeValue(temperatureItem.value) : null
			this.light = lightItem ? this.normalizeValue(lightItem.value) : null
			this.pressure = pressureItem ? this.normalizeValue(pressureItem.value) : null
			this.LED = this.parseBoolean(ledItem ? ledItem.value : false)

			const latestTimestamp = [humidityItem, temperatureItem, lightItem, pressureItem, ledItem]
				.filter(Boolean)
				.map((item) => Number(item.time) || 0)
				.reduce((max, current) => Math.max(max, current), 0)

			this.lastSyncText = this.formatTime(latestTimestamp)
		},
		fetchDeviceData() {
			this.loading = true
			this.errorMessage = ''

			uni.request({
				url: 'https://iot-api.heclouds.com/thingmodel/query-device-property',
				method: 'GET',
				data: {
					product_id: this.apiConfig.productId,
					device_name: this.apiConfig.deviceName
				},
				header: {
					authorization: this.apiConfig.token
				},
				success: (res) => {
					if (!res || !res.data || res.data.code !== 0) {
						this.errorMessage = this.getApiMessage(res, '从 OneNet 获取设备数据失败。')
						return
					}

					const items = res.data.data
					if (!Array.isArray(items)) {
						this.errorMessage = '设备返回格式不符合预期。'
						return
					}

					this.updateDeviceState(items)
				},
				fail: (err) => {
					console.error('Failed to fetch device data', err)
					this.errorMessage = '从 OneNet 获取设备数据失败。'
				},
				complete: () => {
					this.loading = false
				}
			})
		},
		onLEDSwitch(event) {
			const ledValue = event.detail.value
			const previousValue = this.LED
			this.LED = ledValue
			this.ledSubmitting = true
			this.errorMessage = ''

			uni.request({
				url: 'https://iot-api.heclouds.com/thingmodel/set-device-property',
				method: 'POST',
				data: {
					product_id: this.apiConfig.productId,
					device_name: this.apiConfig.deviceName,
					params: {
						led: ledValue
					}
				},
				header: {
					authorization: this.apiConfig.token
				},
				success: (res) => {
					if (!res || !res.data || res.data.code !== 0) {
						this.LED = previousValue
						this.errorMessage = this.getApiMessage(res, '写入 LED 状态失败。')
						uni.showToast({
							title: this.errorMessage,
							icon: 'none'
						})
						return
					}

					uni.showToast({
						title: `LED 已${ledValue ? '开启' : '关闭'}`,
						icon: 'none'
					})
				},
				fail: (err) => {
					console.error('Failed to control LED', err)
					this.LED = previousValue
					this.errorMessage = '写入 LED 状态失败。'
					uni.showToast({
						title: 'LED 控制失败',
						icon: 'none'
					})
				},
				complete: () => {
					this.ledSubmitting = false
				}
			})
		}
	}
}
</script>

<style>
.page {
	min-height: 100vh;
	padding: 18rpx;
	background: radial-gradient(circle at top, #ffffff 0%, #f4f7f8 62%, #eef2f4 100%);
	color: #0f1419;
	font-family: "Microsoft YaHei", "PingFang SC", "Noto Sans SC", "Source Han Sans SC", sans-serif;
}

.page-title {
	padding: 12rpx 0 18rpx;
	text-align: center;
}

.title-main {
	font-size: 36rpx;
	line-height: 1.16;
	font-weight: 800;
	color: #11161c;
}

.title-sub {
	margin-top: 6rpx;
	font-size: 22rpx;
	line-height: 1.2;
	font-weight: 400;
	color: #4a5663;
}

.card-grid {
	width: 100%;
	display: grid;
	grid-template-columns: repeat(2, minmax(0, 1fr));
	gap: 16rpx;
}

.dashboard-card {
	min-height: 252rpx;
	padding: 18rpx 20rpx 18rpx;
	border: 1rpx solid rgba(22, 31, 40, 0.08);
	border-radius: 22rpx;
	background: rgba(255, 255, 255, 0.96);
	box-shadow: 0 10rpx 24rpx rgba(24, 34, 45, 0.11);
	display: flex;
	flex-direction: column;
	align-items: center;
}

.card-icon {
	width: 160rpx;
	height: 132rpx;
	flex: 0 0 auto;
}

.card-label {
	margin-top: 4rpx;
	min-height: 30rpx;
	font-size: 22rpx;
	line-height: 1.25;
	font-weight: 700;
	color: #111821;
	text-align: center;
}

.card-divider {
	width: 100%;
	height: 2rpx;
	margin: 11rpx 0 8rpx;
	background: #d9dee2;
}

.card-value-row {
	min-height: 44rpx;
	display: flex;
	align-items: baseline;
	justify-content: center;
	gap: 8rpx;
	white-space: nowrap;
}

.card-value {
	font-size: 36rpx;
	line-height: 1.1;
	font-weight: 800;
}

.card-unit {
	font-size: 19rpx;
	line-height: 1.2;
	font-weight: 500;
	color: #314050;
}

.control-card {
	cursor: pointer;
}

.control-value-row {
	width: 100%;
	flex-direction: column;
	align-items: center;
	gap: 8rpx;
}

.led-switch {
	transform: scale(0.82);
	transform-origin: center;
}

.status-grid {
	margin-top: 16rpx;
	display: flex;
	gap: 16rpx;
}

.status-card {
	flex: 1;
	min-height: 82rpx;
	padding: 14rpx 18rpx;
	border: 1rpx solid rgba(22, 31, 40, 0.08);
	border-radius: 18rpx;
	background: rgba(255, 255, 255, 0.94);
	box-shadow: 0 8rpx 18rpx rgba(24, 34, 45, 0.08);
	display: flex;
	flex-direction: column;
	justify-content: center;
	gap: 8rpx;
}

.status-label {
	font-size: 18rpx;
	font-weight: 500;
	color: #66717d;
}

.status-text {
	font-size: 24rpx;
	font-weight: 800;
	color: #4f5b66;
}

.status-text.online {
	color: #11823b;
}

.status-text.error {
	color: #e01818;
}

.tone-blue {
	color: #1264c8;
}

.tone-green {
	color: #11823b;
}

.tone-orange {
	color: #f28a00;
}

.tone-red {
	color: #e01818;
}

.tone-purple {
	color: #673ab7;
}

.tone-gray {
	color: #536170;
}

.error-box {
	margin-top: 18rpx;
	padding: 18rpx 22rpx;
	border-radius: 20rpx;
	background: rgba(209, 63, 63, 0.08);
	display: flex;
	flex-direction: column;
	gap: 6rpx;
}

.error-title {
	font-size: 22rpx;
	font-weight: 700;
	color: #bf2f2f;
}

.error-text {
	font-size: 20rpx;
	line-height: 1.6;
	color: #c64a4a;
}

@media (max-width: 560px) {
	.page {
		padding: 12rpx;
	}

	.title-main {
		font-size: 32rpx;
	}

	.title-sub {
		font-size: 20rpx;
	}

	.card-grid {
		gap: 12rpx;
	}

	.dashboard-card {
		min-height: 226rpx;
		padding: 14rpx 18rpx 16rpx;
	}

	.card-icon {
		width: 148rpx;
		height: 116rpx;
	}

	.card-label {
		font-size: 20rpx;
	}

	.card-value {
		font-size: 32rpx;
	}

	.card-unit {
		font-size: 17rpx;
	}
}
</style>
