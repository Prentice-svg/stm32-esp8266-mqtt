<template>
	<view class="page">
		<view class="hero">
			<view class="hero-title">IoT Environment Monitor</view>
			<view class="hero-subtitle">OneNET device dashboard for humidity, temperature, pressure, light, and LED control.</view>
		</view>

		<view class="status-bar">
			<text class="status-label">Status</text>
			<text class="status-value" :class="{ online: !loading && !errorMessage, error: !!errorMessage }">
				{{ statusText }}
			</text>
		</view>

		<view class="sync-row">
			<text class="sync-label">Last Sync</text>
			<text class="sync-value">{{ lastSyncText }}</text>
		</view>

		<view class="card-grid">
			<view class="metric-card">
				<view class="metric-head">
					<view class="metric-name">Humidity</view>
					<image class="metric-icon" src="/static/humidity.png" mode="aspectFit"></image>
				</view>
				<view class="metric-value">{{ humidityDisplay }}</view>
				<view class="metric-unit">%RH</view>
			</view>

			<view class="metric-card">
				<view class="metric-head">
					<view class="metric-name">Temperature</view>
					<image class="metric-icon" src="/static/temperature.png" mode="aspectFit"></image>
				</view>
				<view class="metric-value">{{ temperatureDisplay }}</view>
				<view class="metric-unit">degC</view>
			</view>

			<view class="metric-card">
				<view class="metric-head">
					<view class="metric-name">Light</view>
					<image class="metric-icon" src="/static/light.png" mode="aspectFit"></image>
				</view>
				<view class="metric-value">{{ lightDisplay }}</view>
				<view class="metric-unit">%</view>
			</view>

			<view class="metric-card">
				<view class="metric-head">
					<view class="metric-name">Pressure</view>
					<image class="metric-icon" src="/static/pressure.png" mode="aspectFit"></image>
				</view>
				<view class="metric-value">{{ pressureDisplay }}</view>
				<view class="metric-unit">hPa</view>
			</view>

			<view class="metric-card led-card">
				<view class="metric-head">
					<view class="metric-name">LED</view>
					<image class="metric-icon" src="/static/LED.png" mode="aspectFit"></image>
				</view>
				<switch :checked="LED" :disabled="ledSubmitting" @change="onLEDSwitch" color="#1d7dfa" />
				<view class="led-text">{{ LED ? 'ON' : 'OFF' }}</view>
			</view>
		</view>

		<view v-if="errorMessage" class="error-box">
			{{ errorMessage }}
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
		statusText() {
			if (this.errorMessage) {
				return 'Request Failed'
			}
			if (this.loading) {
				return 'Syncing'
			}
			return 'Connected'
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
		getPropertyValue(items, identifier) {
			const item = items.find((entry) => entry.identifier === identifier)
			return item ? item.value : null
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
						this.errorMessage = this.getApiMessage(res, 'Failed to fetch device data from OneNET.')
						return
					}

					const items = res.data.data
					if (!Array.isArray(items)) {
						this.errorMessage = 'Unexpected device response.'
						return
					}

					this.updateDeviceState(items)
				},
				fail: (err) => {
					console.error('Failed to fetch device data', err)
					this.errorMessage = 'Failed to fetch device data from OneNET.'
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
						this.errorMessage = this.getApiMessage(res, 'Failed to update LED state.')
						uni.showToast({
							title: this.errorMessage,
							icon: 'none'
						})
						return
					}

					uni.showToast({
						title: `LED ${ledValue ? 'ON' : 'OFF'}`,
						icon: 'none'
					})
				},
				fail: (err) => {
					console.error('Failed to control LED', err)
					this.LED = previousValue
					this.errorMessage = 'Failed to update LED state.'
					uni.showToast({
						title: 'LED update failed',
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
	background: linear-gradient(180deg, #eef6ff 0%, #f8fbff 35%, #ffffff 100%);
}

.hero {
	padding: 28rpx 24rpx;
	border-radius: 24rpx;
	background: linear-gradient(135deg, #143a52 0%, #1d7dfa 100%);
	box-shadow: 0 18rpx 40rpx rgba(29, 125, 250, 0.18);
}

.hero-title {
	color: #ffffff;
	font-size: 34rpx;
	font-weight: 700;
}

.hero-subtitle {
	margin-top: 10rpx;
	color: rgba(255, 255, 255, 0.82);
	font-size: 22rpx;
	line-height: 1.5;
}

.status-bar {
	margin-top: 18rpx;
	padding: 18rpx 20rpx;
	border-radius: 18rpx;
	background: #ffffff;
	display: flex;
	justify-content: space-between;
	align-items: center;
	box-shadow: 0 10rpx 28rpx rgba(20, 58, 82, 0.08);
	gap: 20rpx;
}

.status-label {
	color: #5d7285;
	font-size: 22rpx;
}

.status-value {
	color: #18794e;
	font-size: 22rpx;
	font-weight: 600;
	text-align: right;
	flex: 1;
}

.status-value.error {
	color: #d13f3f;
}

.status-value.online {
	color: #18794e;
}

.card-grid {
	margin-top: 14rpx;
	display: flex;
	flex-wrap: wrap;
	justify-content: space-between;
	column-gap: 10rpx;
	row-gap: 12rpx;
}

.sync-row {
	margin-top: 12rpx;
	padding: 0 4rpx;
	display: flex;
	justify-content: space-between;
	align-items: center;
}

.sync-label,
.sync-value {
	font-size: 20rpx;
	color: #6e8396;
}

.metric-card {
	width: calc(50% - 5rpx);
	min-height: 176rpx;
	padding: 18rpx;
	border-radius: 20rpx;
	background: #ffffff;
	box-shadow: 0 14rpx 32rpx rgba(15, 40, 60, 0.08);
	display: flex;
	flex-direction: column;
	justify-content: space-between;
	box-sizing: border-box;
}

.metric-head {
	display: flex;
	align-items: center;
	justify-content: space-between;
}

.metric-name {
	font-size: 24rpx;
	font-weight: 600;
	color: #29465b;
}

.metric-icon {
	width: 58rpx;
	height: 58rpx;
}

.metric-value {
	margin-top: 12rpx;
	font-size: 38rpx;
	font-weight: 700;
	color: #132534;
}

.metric-unit {
	margin-top: 6rpx;
	font-size: 18rpx;
	color: #71879a;
}

.led-card {
	align-items: flex-start;
}

.led-text {
	margin-top: 12rpx;
	font-size: 24rpx;
	font-weight: 700;
	color: #132534;
}

.error-box {
	margin-top: 24rpx;
	padding: 22rpx 24rpx;
	border-radius: 20rpx;
	background: #fff1f1;
	color: #bb2d3b;
	font-size: 24rpx;
	line-height: 1.5;
}

@media (max-width: 560px) {
	.metric-card {
		min-height: 168rpx;
		padding: 16rpx;
	}
}
</style>
