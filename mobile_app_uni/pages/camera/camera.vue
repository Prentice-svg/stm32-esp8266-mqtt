<template>
	<view class="page">
		<AppHeader title="摄像头" subtitle="ESP32-S3-CAM 实时监控">
			<template #right>
				<view class="back-btn" @tap="goBack">返回</view>
			</template>
		</AppHeader>

		<view class="camera-shell">
			<web-view v-if="cameraUrl" :src="cameraUrl"></web-view>
			<view v-else class="empty-camera">
				<text class="empty-title">未配置摄像头地址</text>
				<text class="empty-text">请先在设置页填写 ESP32-S3-CAM IP</text>
			</view>
		</view>
	</view>
</template>

<script>
import AppHeader from '../../components/AppHeader.vue'

export default {
	components: {
		AppHeader
	},
	data() {
		return {
			cameraUrl: ''
		}
	},
	onLoad(options) {
		const rawUrl = options && options.url ? decodeURIComponent(options.url) : ''
		this.cameraUrl = rawUrl
	},
	methods: {
		goBack() {
			uni.navigateBack()
		}
	}
}
</script>

<style>
.page {
	min-height: 100vh;
	padding: 32rpx 32rpx 0;
	background: #F6F9FC;
	box-sizing: border-box;
}

.back-btn {
	padding: 12rpx 20rpx;
	border-radius: 999rpx;
	background: rgba(37, 99, 235, 0.12);
	color: #2563EB;
	font-size: 24rpx;
	font-weight: 850;
}

.camera-shell {
	position: fixed;
	left: 0;
	right: 0;
	top: 180rpx;
	bottom: 0;
	background: #0F172A;
}

web-view {
	width: 100%;
	height: 100%;
}

.empty-camera {
	height: 100%;
	display: flex;
	flex-direction: column;
	align-items: center;
	justify-content: center;
	gap: 14rpx;
	color: #FFFFFF;
}

.empty-title {
	font-size: 30rpx;
	font-weight: 900;
}

.empty-text {
	font-size: 24rpx;
	color: rgba(255, 255, 255, 0.72);
}
</style>
