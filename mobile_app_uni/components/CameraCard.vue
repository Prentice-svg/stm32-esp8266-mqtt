<template>
	<view class="camera-card">
		<view class="camera-head">
			<view>
				<text class="camera-title">摄像头</text>
				<text class="camera-subtitle">{{ subtitle }}</text>
			</view>
			<view class="camera-status" :class="{ active: isStreaming }">
				<text class="status-dot"></text>
				<text>{{ isStreaming ? '监控中' : '待机' }}</text>
			</view>
		</view>

		<view class="camera-view">
			<image
				v-if="displayUrl"
				class="camera-image"
				:src="displayUrl"
				mode="aspectFill"
				@error="handleImageError"
			/>
			<view v-else class="camera-placeholder">
				<image class="placeholder-icon" src="/static/icons/camera_status_system_d.png" mode="aspectFit" />
				<text class="placeholder-title">未开始预览</text>
				<text class="placeholder-text">在设置页填写 ESP32-S3-CAM IP 后启动</text>
			</view>
		</view>

		<view class="camera-actions">
			<button class="camera-btn primary" @tap="toggleStream">
				<text class="btn-dot" :class="{ stop: isStreaming }"></text>
				<text>{{ isStreaming ? '停止' : '开始' }}</text>
			</button>
			<button class="camera-btn secondary" @tap="captureSnapshot">
				<text>📷</text>
				<text>截图</text>
			</button>
			<button class="camera-btn secondary" @tap="openCameraPage">
				<text>↗</text>
				<text>打开</text>
			</button>
		</view>
	</view>
</template>

<script>
export default {
	name: 'CameraCard',
	props: {
		cameraIp: {
			type: String,
			default: ''
		},
		enabled: {
			type: Boolean,
			default: true
		}
	},
	data() {
		return {
			isStreaming: false,
			snapshotVersion: 0,
			lastMode: 'idle'
		}
	},
	computed: {
		normalizedIp() {
			return String(this.cameraIp || '').trim()
		},
		baseUrl() {
			if (!this.normalizedIp) return ''
			if (/^https?:\/\//i.test(this.normalizedIp)) {
				return this.normalizedIp.replace(/\/$/, '')
			}
			return `http://${this.normalizedIp}`
		},
		streamUrl() {
			return this.baseUrl ? `${this.baseUrl}/stream` : ''
		},
		captureUrl() {
			if (!this.baseUrl) return ''
			return `${this.baseUrl}/capture?t=${this.snapshotVersion}`
		},
		displayUrl() {
			if (!this.enabled || !this.baseUrl) return ''
			if (this.isStreaming) return this.streamUrl
			if (this.lastMode === 'capture') return this.captureUrl
			return ''
		},
		subtitle() {
			if (!this.enabled) return '未启用'
			if (!this.baseUrl) return '未配置 IP'
			return this.baseUrl
		}
	},
	watch: {
		cameraIp() {
			this.isStreaming = false
			this.lastMode = 'idle'
		},
		enabled(value) {
			if (!value) {
				this.isStreaming = false
				this.lastMode = 'idle'
			}
		}
	},
	methods: {
		toggleStream() {
			if (!this.ensureReady()) return
			this.isStreaming = !this.isStreaming
			this.lastMode = this.isStreaming ? 'stream' : 'idle'
		},
		captureSnapshot() {
			if (!this.ensureReady()) return
			this.isStreaming = false
			this.snapshotVersion += 1
			this.lastMode = 'capture'
		},
		openCameraPage() {
			if (!this.ensureReady()) return
			// App 端用内置浏览器打开；H5 端保留当前页面体验。
			// #ifdef APP-PLUS
			plus.runtime.openURL(this.baseUrl)
			// #endif
			// #ifndef APP-PLUS
			window.open(this.baseUrl, '_blank')
			// #endif
		},
		ensureReady() {
			if (!this.enabled) {
				uni.showToast({
					title: '摄像头未启用',
					icon: 'none'
				})
				return false
			}
			if (!this.baseUrl) {
				uni.showToast({
					title: '请先配置摄像头 IP',
					icon: 'none'
				})
				return false
			}
			return true
		},
		handleImageError() {
			uni.showToast({
				title: '摄像头画面加载失败',
				icon: 'none'
			})
		}
	}
}
</script>

<style>
.camera-card {
	margin-top: 24rpx;
	padding: 28rpx;
	border-radius: 32rpx;
	background: #FFFFFF;
	box-shadow: 0 16rpx 38rpx rgba(15, 23, 42, 0.065);
	border: 1rpx solid rgba(226, 232, 240, 0.72);
}

.camera-head {
	display: flex;
	align-items: flex-start;
	justify-content: space-between;
	gap: 20rpx;
	margin-bottom: 22rpx;
}

.camera-title,
.camera-subtitle {
	display: block;
}

.camera-title {
	font-size: 30rpx;
	font-weight: 900;
	color: #0F172A;
}

.camera-subtitle {
	margin-top: 6rpx;
	font-size: 22rpx;
	color: #94A3B8;
}

.camera-status {
	padding: 10rpx 16rpx;
	border-radius: 999rpx;
	background: #F1F5F9;
	color: #64748B;
	display: flex;
	align-items: center;
	gap: 8rpx;
	font-size: 22rpx;
	font-weight: 850;
	white-space: nowrap;
}

.camera-status.active {
	background: rgba(34, 197, 94, 0.14);
	color: #16A34A;
}

.status-dot {
	width: 10rpx;
	height: 10rpx;
	border-radius: 50%;
	background: currentColor;
}

.camera-view {
	width: 100%;
	height: 360rpx;
	border-radius: 24rpx;
	background: #EAF1F8;
	overflow: hidden;
	position: relative;
}

.camera-image {
	width: 100%;
	height: 100%;
	display: block;
	background: #0F172A;
}

.camera-placeholder {
	height: 100%;
	display: flex;
	flex-direction: column;
	align-items: center;
	justify-content: center;
	gap: 12rpx;
}

.placeholder-icon {
	width: 72rpx;
	height: 72rpx;
	opacity: 0.78;
}

.placeholder-title {
	font-size: 26rpx;
	font-weight: 900;
	color: #334155;
}

.placeholder-text {
	font-size: 22rpx;
	color: #94A3B8;
}

.camera-actions {
	margin-top: 22rpx;
	display: flex;
	gap: 16rpx;
}

.camera-btn {
	flex: 1;
	margin: 0;
	padding: 0;
	height: 72rpx;
	line-height: 72rpx;
	border-radius: 999rpx;
	font-size: 24rpx;
	font-weight: 900;
	display: flex;
	align-items: center;
	justify-content: center;
	gap: 8rpx;
}

.camera-btn::after {
	border: 0;
}

.camera-btn.primary {
	background: linear-gradient(135deg, #2563EB 0%, #14B8A6 100%);
	color: #FFFFFF;
}

.camera-btn.secondary {
	background: #F8FAFC;
	color: #334155;
	border: 1rpx solid #E2E8F0;
}

.btn-dot {
	width: 14rpx;
	height: 14rpx;
	border-radius: 50%;
	background: #EF4444;
}

.btn-dot.stop {
	border-radius: 4rpx;
}
</style>
